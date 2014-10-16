#ifndef GRUB_SPEAKER_HEADER
#define GRUB_SPEAKER_HEADER 1

#include <grub/cpu/io.h>
#include <grub/i386/pit.h>

/* The frequency of the PIT clock.  */
#define GRUB_SPEAKER_PIT_FREQUENCY		0x1234dd

static inline void
grub_speaker_beep_off (void)
{
  unsigned char status;

  status = grub_inb (GRUB_PIT_SPEAKER_PORT);
  grub_outb (status & ~(GRUB_PIT_SPK_TMR2 | GRUB_PIT_SPK_DATA),
	     GRUB_PIT_SPEAKER_PORT);
}

static inline void
grub_speaker_beep_on (grub_uint16_t pitch)
{
  unsigned char status;
  unsigned int counter;

  if (pitch < 20)
    pitch = 20;
  else if (pitch > 20000)
    pitch = 20000;

  counter = GRUB_SPEAKER_PIT_FREQUENCY / pitch;

  /* Program timer 2.  */
  grub_outb (GRUB_PIT_CTRL_SELECT_2
	     | GRUB_PIT_CTRL_READLOAD_WORD
	     | GRUB_PIT_CTRL_SQUAREWAVE_GEN
	     | GRUB_PIT_CTRL_COUNT_BINARY, GRUB_PIT_CTRL);
  grub_outb (counter & 0xff, GRUB_PIT_COUNTER_2);		/* LSB */
  grub_outb ((counter >> 8) & 0xff, GRUB_PIT_COUNTER_2);	/* MSB */

  /* Start speaker.  */
  status = grub_inb (GRUB_PIT_SPEAKER_PORT);
  grub_outb (status | GRUB_PIT_SPK_TMR2 | GRUB_PIT_SPK_DATA,
	     GRUB_PIT_SPEAKER_PORT);
}

#endif
