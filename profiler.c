#include "profiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define strmatch(a, b) (strcmp((a), (b)) == 0)

#define TERMINATE(...)                                                         \
  fprintf(stderr, __VA_ARGS__);                                                \
  fprintf(stderr, " %s:%d\n", __FILE__, __LINE__);                             \
  exit(EXIT_FAILURE);

// Internally start the profiling timer
void profiler_start_timer(struct Profile* profile) {
#ifdef __APPLE__
  profile->profiler_start = mach_absolute_time();
#else
  clock_gettime(CLOCK_MONOTONIC, &profile->profiler_start);
#endif
}

// Internally end the profiling timer and store results
void profiler_end_timer(struct Profile* profile, const char* entry_name) {
#ifdef __APPLE__
  profile->profiler_end = mach_absolute_time();
#else
  clock_gettime(CLOCK_MONOTONIC, &profile->profiler_end);
#endif

  // Check if an entry exists
  int ii;
  for (ii = 0; ii < profile->profiler_entry_count; ++ii) {
    if (strmatch(profile->profiler_entries[ii].name, entry_name)) {
      break;
    }
  }

  // Don't overrun
  if (ii >= PROFILER_MAX_ENTRIES) {
    TERMINATE("Attempted to profile too many entries, maximum is %d\n",
              PROFILER_MAX_ENTRIES);
  }

  // Create new entry
  if (ii == profile->profiler_entry_count) {
    profile->profiler_entry_count++;
    strcpy(profile->profiler_entries[ii].name, entry_name);
    profile->profiler_entries[ii].time = 0.0;
  }

// Update number of calls and time
#ifdef __APPLE__
  double elapsed = (profile->profiler_end - profile->profiler_start) * 1.0E-9;
#else
  double elapsed =
      (profile->profiler_end.tv_sec - profile->profiler_start.tv_sec) +
      (profile->profiler_end.tv_nsec - profile->profiler_start.tv_nsec) *
          1.0E-9;
#endif

  profile->profiler_entries[ii].time += elapsed;
  profile->profiler_entries[ii].calls++;
}

// Print the profiling results to output
void profiler_print_full_profile(struct Profile* profile) {
  printf("\n-------------------------------------------------------------\n");
  printf("\nProfiling Results:\n\n");
  printf("%-39s%8s%28s\n", "Kernel Name", "Calls", "Runtime (s)");

  double total_elapsed_time = 0.0;
  for (int ii = 0; ii < profile->profiler_entry_count; ++ii) {
    total_elapsed_time += profile->profiler_entries[ii].time;
    printf("%-39s%8d%28.9F\n", profile->profiler_entries[ii].name,
           profile->profiler_entries[ii].calls,
           profile->profiler_entries[ii].time);
  }

  printf("\nTotal elapsed time: %.9Fs, entries * are excluded.\n",
         total_elapsed_time);
  printf("\n-------------------------------------------------------------\n\n");
}

// Prints profile without extra details
void profiler_print_simple_profile(struct Profile* profile) {
  for (int ii = 0; ii < profile->profiler_entry_count; ++ii) {
    printf("\033[1m\033[30m%s\033[0m: %.8lfs (%d calls)\n",
           profile->profiler_entries[ii].name,
           profile->profiler_entries[ii].time,
           profile->profiler_entries[ii].calls);
  }
}

// Gets an individual profile entry
struct ProfileEntry profiler_get_profile_entry(struct Profile* profile,
                                               const char* entry_name) {

  for (int ii = 0; ii < profile->profiler_entry_count; ++ii) {
    if (strmatch(profile->profiler_entries[ii].name, entry_name)) {
      return profile->profiler_entries[ii];
    }
  }

  TERMINATE("Attempted to retrieve missing profile entry %s\n", entry_name);
}
