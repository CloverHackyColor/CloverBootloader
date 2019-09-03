#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

int
main (int argc, char **argv)
{
  struct rusage buf;

  if (argc > 1)
    {
      system (argv[1]);

      if (getrusage (RUSAGE_CHILDREN, &buf ))
        {
          perror ("getrusage");
          return 1;
        }
    }
  else
    {
      if (getrusage (RUSAGE_SELF, &buf ))
        {
          perror ("getrusage");
          return 1;
        }
    }

  printf ("ru_utime   = %ld.%06ld\n",
          buf.ru_utime.tv_sec, buf.ru_utime.tv_usec); 
  printf ("ru_stime   = %ld.%06ld\n",
          buf.ru_stime.tv_sec, buf.ru_stime.tv_usec);
  printf ("ru_maxrss  = %ld\n", buf.ru_maxrss   );
  printf ("ru_ixrss   = %ld\n", buf.ru_ixrss    );
  printf ("ru_idrss   = %ld\n", buf.ru_idrss    );
  printf ("ru_isrss   = %ld\n", buf.ru_isrss    );
  printf ("ru_minflt  = %ld\n", buf.ru_minflt   );
  printf ("ru_majflt  = %ld\n", buf.ru_majflt   );
  printf ("ru_nswap   = %ld\n", buf.ru_nswap    );
  printf ("ru_inblock = %ld\n", buf.ru_inblock  );
  printf ("ru_oublock = %ld\n", buf.ru_oublock  );
  printf ("ru_msgsnd  = %ld\n", buf.ru_msgsnd   );
  printf ("ru_msgrcv  = %ld\n", buf.ru_msgrcv   );
  printf ("ru_nsignals= %ld\n", buf.ru_nsignals );
  printf ("ru_nvcsw   = %ld\n", buf.ru_nvcsw    );
  printf ("ru_nivcsw  = %ld\n", buf.ru_nivcsw   );

  fprintf (stderr, "ru_utime   ru_stime   ru_minflt  ru_nccsw  ru_nivcsw\n");
  fprintf (stderr, "%ld.%06ld  %ld.%06ld  %5ld       %5ld      %5ld\n");


  return 0;
}


/* Codesnippet for debugging in random.c. */
#if 0
static void
collect_rusage_stats (struct rusage *rb)
{
  static int idx;
  static struct rusage buf[100];
  
  if (!rb)
    {
      int i;

      fprintf (stderr, "ru_utime   ru_stime   ru_minflt  ru_nvcsw  ru_nivcsw\n");
      for (i=0; i < idx; i++)
        fprintf (stderr, "%ld.%06ld   %ld.%06ld %5ld       %5ld      %5ld\n",
                 buf[i].ru_utime.tv_sec, buf[i].ru_utime.tv_usec, 
                 buf[i].ru_stime.tv_sec, buf[i].ru_stime.tv_usec, 
                 buf[i].ru_minflt, 
                 buf[i].ru_nvcsw,
                 buf[i].ru_nivcsw);
    }      
  else if (idx < DIM(buf))
    {
      buf[idx++] = *rb;
    }
}
#endif
/*
 void
 _gcry_random_dump_stats()
 {
@@ -233,8 +261,11 @@
                  rndstats.naddbytes, rndstats.addbytes,
        rndstats.mixkey, rndstats.ngetbytes1, rndstats.getbytes1,
                    rndstats.ngetbytes2, rndstats.getbytes2 );
+
+    collect_rusage_stats (NULL);
 }

========

     getrusage (RUSAGE_SELF, &buf );
+    collect_rusage_stats (&buf);
     add_randomness( &buf, sizeof buf, 1 );
     memset( &buf, 0, sizeof buf );
   }
 
*/


