/***** jack.record.c - (c) rohan drape, 2003-2004 *****/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "common/failure.h"
#include "common/file.h"
#include "common/jack-client.h"
#include "common/jack-port.h"
#include "common/jack-ringbuffer.h"
#include "common/memory.h"
#include "common/observe-signal.h"
#include "common/print.h"
#include "common/signal-interleave.h"
#include "common/sound-file.h"

typedef struct timemark
{

  int value;
  struct timemark *next;

} timemark_t;

typedef struct
{
  int buffer_bytes;
  int buffer_samples;
  int buffer_frames;
  int minimal_frames;
  float timer_seconds;
  int timer_frames;
  int timer_counter;
  float sample_rate;
  float *d_buffer;
  float *j_buffer;
  float *u_buffer;
  int file_format;
  SNDFILE **sound_file;
  int multiple_sound_files;
  int channels;
  jack_port_t **input_port;
  float **in;
  jack_ringbuffer_t *ring_buffer;
  pthread_t disk_thread;
  int pipe[2];
  char *filetemplate;
  int loop_files;
  timemark_t *timer_timemarks;
}
jackrecord_t;

/* Write data to disk. */

void
jackrecord_write_to_disk (jackrecord_t * d, int nframes)
{
  if (d->multiple_sound_files)
    {
      float *p = d->u_buffer;
      signal_uninterleave (p, d->d_buffer, nframes, d->channels);
      int i;
      for (i = 0; i < d->channels; i++)
	{
	  xsf_write_float (d->sound_file[i], p, (sf_count_t) nframes);
	  p += nframes;
	}
    }
  else
    {
      int nsamples = nframes * d->channels;
      xsf_write_float (d->sound_file[0], d->d_buffer, (sf_count_t) nsamples);
    }
}

char *
jackrecord_replaceN (char *filename, int size, char *target,
		     int numeric_value)
{
  char numeric_string[3];
  snprintf (numeric_string, 3, "%02d", numeric_value);

  int filename_length = strlen (filename);
  int length = filename_length < size ? filename_length : size;

  int position;
  for (position = 0; position < length; position++)
    {
      char current = filename[position];

      if (position + 1 < length && current == '%'
	  && filename[position + 1] == 'N')
	{
	  target[position] = numeric_string[0];
	  target[position + 1] = numeric_string[1];
	  position++;
	}
      else
	{
	  target[position] = current;
	}
    }
  target[length] = '\0';
  return target;
}

void
jackrecord_open_files (jackrecord_t * d)
{
  SF_INFO sfinfo;
  sfinfo.samplerate = (int) d->sample_rate;
  sfinfo.frames = 0;
  sfinfo.format = d->file_format;

  time_t now = time (NULL);

  if (d->multiple_sound_files)
    {
      sfinfo.channels = 1;
      int i;
      for (i = 0; i < d->channels; i++)
	{
	  char name[512];
	  jackrecord_replaceN (d->filetemplate, 512, name, i);

	  char timed_name[512];
	  strftime (timed_name, 512, name, localtime (&now));
	  d->sound_file[i] = xsf_open (timed_name, SFM_WRITE, &sfinfo);
	}
    }
  else
    {
      sfinfo.channels = d->channels;

      char name[512];
      strftime (name, 512, d->filetemplate, localtime (&now));
      d->sound_file[0] = xsf_open (name, SFM_WRITE, &sfinfo);
    }
}

void
jackrecord_close_files (jackrecord_t * d)
{
  if (d->multiple_sound_files)
    {
      int i;
      for (i = 0; i < d->channels; i++)
	{
	  sf_close (d->sound_file[i]);
	}
    }
  else
    {
      sf_close (d->sound_file[0]);
    }
}

/* Wait for data on the ringbuffer and write to disk. */

void *
jackrecord_disk_thread_procedure (void *PTR)
{
  jackrecord_t *d = (jackrecord_t *) PTR;

  timemark_t *next_timemark = d->timer_timemarks;
  if (next_timemark != NULL) {
		time_t now = time (NULL);
		struct tm *local_now = localtime (&now);

		int now_minutes = local_now->tm_min;

		while (next_timemark->value < next_timemark->next->value && now_minutes < next_timemark->value) {
			next_timemark = next_timemark->next;
		}
	}

  while (!observe_end_of_process ())
    {

      /* Wait for data at the ring buffer. */

      int nbytes = d->minimal_frames * sizeof (float) * d->channels;
      nbytes = jack_ringbuffer_wait_for_read (d->ring_buffer, nbytes,
					      d->pipe[0]);

      /* Drop excessive space to not overflow the local buffer. */

      if (nbytes > d->buffer_bytes)
	{
	  eprintf ("jack.record: impossible condition, read space.\n");
	  nbytes = d->buffer_bytes;
	}

      /* Read data from the ring buffer. */

      jack_ringbuffer_read (d->ring_buffer, (char *) d->d_buffer, nbytes);

      /* Do write operation.  The sample count *must* be an integral
         number of frames. */

      int nframes = (nbytes / sizeof (float)) / d->channels;
      jackrecord_write_to_disk (d, nframes);

      /* Handle timer */

      d->timer_counter += nframes;
      int timer_ended = 0;

      if (d->timer_frames > 0 && d->timer_counter >= d->timer_frames)
	{
	  timer_ended = 1;
	}
      else if (next_timemark != NULL)
	{
	  time_t now = time (NULL);
	  struct tm *local_now = localtime (&now);

	  int now_minutes = local_now->tm_min;
	  if (now_minutes == next_timemark->value)
	    {
	      printf ("timemark detected: %d\n", now_minutes);
	      timer_ended = 1;
	      next_timemark = next_timemark->next;
	    }
	}

      if (timer_ended)
	{
	  if (d->loop_files)
	    {
	      jackrecord_close_files (d);
	      jackrecord_open_files (d);
	      d->timer_counter = 0;
	    }
	  else
	    {
	      return NULL;
	    }
	}
    }
  return NULL;
}

/* Write data from the JACK input ports to the ring buffer.  If the
   disk thread is late, ie. the ring buffer is full, print an error
   and halt the client.  */

int
jackrecord_process (jack_nframes_t nframes, void *PTR)
{
  jackrecord_t *d = (jackrecord_t *) PTR;
  int nsamples = nframes * d->channels;
  int nbytes = nsamples * sizeof (float);

  /* Get port data buffers. */

  int i;
  for (i = 0; i < d->channels; i++)
    {
      d->in[i] = (float *) jack_port_get_buffer (d->input_port[i], nframes);
    }

  /* Check period size is workable. If the buffer is large , ie 4096
     frames , this should never be of practical concern. */

  if (nbytes >= d->buffer_bytes)
    {
      eprintf ("jack.record: period size exceeds limit\n");
      FAILURE;
      return 1;
    }

  /* Check that there is adequate space in the ringbuffer. */

  int space = (int) jack_ringbuffer_write_space (d->ring_buffer);
  if (space < nbytes)
    {
      eprintf ("jack.record: overflow error, %d > %d\n", nbytes, space);
      FAILURE;
      return 1;
    }

  /* Interleave input to buffer and copy into ringbuffer. */

  signal_interleave_to (d->j_buffer,
			(const float **) d->in, nframes, d->channels);
  int err = jack_ringbuffer_write (d->ring_buffer,
				   (char *) d->j_buffer,
				   (size_t) nbytes);
  if (err != nbytes)
    {
      eprintf ("jack.record: error writing to ringbuffer, %d != %d\n",
	       err, nbytes);
      FAILURE;
      return 1;
    }

  /* Poke the disk thread to indicate data is on the ring buffer. */

  char b = 1;
  xwrite (d->pipe[1], &b, 1);

  return 0;
}

struct timemark *
jackrecord_create_timemarks (char *definition)
{
  char *token = strtok (definition, ",");

  struct timemark *first = NULL;
  struct timemark *current = NULL;

  while (token != NULL)
    {
      int time_mark_value = strtol (token, NULL, 0);

      if (time_mark_value < 0 || time_mark_value >= 60)
	{
	  eprintf ("jack.record: invalid timemark: %d\n", time_mark_value);
	  return NULL;
	}

      if (current != NULL)
	{
	  if (time_mark_value <= current->value)
	    {
	      eprintf ("jack.record: invalid timemark sequence: %d < %d\n",
		       time_mark_value, current->value);
	      return NULL;
	    }
	}

      struct timemark *new = xmalloc (sizeof (struct timemark *));
      new->value = time_mark_value;
      new->next = NULL;

      if (current == NULL)
	{
	  first = new;
	}
      else
	{
	  current->next = new;
	}

      current = new;

      token = strtok (NULL, ",");
    }
  current->next = first;

  return first;
}

void
jackrecord_usage (void)
{
  eprintf ("Usage: jack.record [ options ] sound-file\n");
  eprintf ("    -b N : Ring buffer size in frames (default=4096).\n");
  eprintf ("    -f N : File format (default=0x10006).\n");
  eprintf ("    -m N : Minimal disk read size in frames (default=32).\n");
  eprintf ("    -n N : Number of channels (default=2).\n");
  eprintf ("    -s   : Write to multiple single channel sound files.\n");
  eprintf
    ("    -t N : Set the time length to record at N seconds (default=-1).\n");
  eprintf
    ("    -k M1,M2,.. : Set the time marks for the record, 0 <= Mx < 60 represent minutes in the hour.\n");
  eprintf ("    -l : Restart with new file(s) when the timer is ended\n");
  FAILURE;
}

int
main (int argc, char *argv[])
{
  observe_signals ();
  jackrecord_t d;
  d.buffer_frames = 4096;
  d.minimal_frames = 32;
  d.channels = 2;
  d.timer_seconds = -1.0;
  d.timer_counter = 0;
  d.file_format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
  d.multiple_sound_files = 0;
  int c;
  while ((c = getopt (argc, argv, "b:f:hk:lm:n:st:")) != -1)
    {
      switch (c)
	{
	case 'b':
	  d.buffer_frames = (int) strtol (optarg, NULL, 0);
	  break;
	case 'f':
	  d.file_format = (int) strtol (optarg, NULL, 0);
	  break;
	case 'h':
	  jackrecord_usage ();
	  break;
	case 'm':
	  d.minimal_frames = (int) strtol (optarg, NULL, 0);
	  break;
	case 'n':
	  d.channels = (int) strtol (optarg, NULL, 0);
	  break;
	case 'l':
	  d.loop_files = 1;
	  break;
	case 's':
	  d.multiple_sound_files = 1;
	  break;
	case 't':
	  d.timer_seconds = (float) strtod (optarg, NULL);
	  break;
	case 'k':
	  d.timer_timemarks = jackrecord_create_timemarks (optarg);
	  if (d.timer_timemarks == NULL)
	    {
	      FAILURE;
	    }
	  break;
	default:
	  eprintf ("jack.record: illegal option , %c\n", c);
	  jackrecord_usage ();
	  break;
	}
    }
  if (optind != argc - 1)
    {
      jackrecord_usage ();
    }

  /* Allocate channel based data. */

  if (d.channels < 1)
    {
      eprintf ("jack.record: illegal number of channels requested: %d\n",
	       d.channels);
      FAILURE;
    }

  d.in = xmalloc (d.channels * sizeof (float *));
  d.sound_file = xmalloc (d.channels * sizeof (SNDFILE *));
  d.input_port = xmalloc (d.channels * sizeof (jack_port_t *));

  /* Connect to JACK. */

  jack_client_t *client = jack_client_unique ("jack.record");
  jack_set_error_function (jack_client_minimal_error_handler);
  jack_on_shutdown (client, jack_client_minimal_shutdown_handler, 0);
  jack_set_process_callback (client, jackrecord_process, &d);
  d.sample_rate = jack_get_sample_rate (client);

  /* Setup timer. */

  if (d.timer_seconds < 0.0)
    {
      d.timer_frames = -1;
    }
  else
    {
      d.timer_frames = d.timer_seconds * d.sample_rate;
    }

  if (d.timer_timemarks != NULL && d.timer_seconds > -1)
    {
      eprintf ("jack.record: -t and -k can't be used both\n");
      FAILURE;
    }

  d.filetemplate = argv[optind];
  if (d.multiple_sound_files)
    {
      if (!strstr (d.filetemplate, "%N"))
	{
	  eprintf ("jack.record: illegal template , '%s'\n", d.filetemplate);
	  jackrecord_usage ();
	}
    }

  /* Create sound file. */
  jackrecord_open_files (&d);

  /* Allocate buffers. */

  d.buffer_samples = d.buffer_frames * d.channels;
  d.buffer_bytes = d.buffer_samples * sizeof (float);
  d.d_buffer = xmalloc (d.buffer_bytes);
  d.j_buffer = xmalloc (d.buffer_bytes);
  d.u_buffer = xmalloc (d.buffer_bytes);
  d.ring_buffer = jack_ringbuffer_create (d.buffer_bytes);

  /* Create communication pipe. */

  xpipe (d.pipe);

  /* Start disk thread. */

  pthread_create (&(d.disk_thread),
		  NULL, jackrecord_disk_thread_procedure, &d);

  /* Create input ports and activate client. */

  jack_port_make_standard (client, d.input_port, d.channels, false);
  jack_client_activate (client);

  /* Wait for disk thread to end , which it does when it reaches the
     end of the file or is interrupted. */

  pthread_join (d.disk_thread, NULL);

  /* Close sound file, free ring buffer, close JACK connection, close
     pipe, free data buffers, indicate success. */

  jack_client_close (client);

  jackrecord_close_files (&d);

  jack_ringbuffer_free (d.ring_buffer);
  close (d.pipe[0]);
  close (d.pipe[1]);
  free (d.d_buffer);
  free (d.j_buffer);
  free (d.u_buffer);
  free (d.in);
  free (d.input_port);
  free (d.sound_file);
  return EXIT_SUCCESS;
}
