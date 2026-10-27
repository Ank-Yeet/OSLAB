#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_FRAMES 32
#define MAX_REFS 128

// Helper to print a percentage value formatted to two decimal places
static void
print_ratio(int hits, int total)
{
  if(total == 0){
    printf(1, "0.00%%");
    return;
  }
  int whole = (hits * 100) / total;
  int frac = ((hits * 10000) / total) % 100;
  if(frac < 10)
    printf(1, "%d.0%d%%", whole, frac);
  else
    printf(1, "%d.%d%%", whole, frac);
}

// Print the current state of frames
static void
print_frames(int frames[], int num_frames)
{
  printf(1, "[");
  for(int i = 0; i < num_frames; i++){
    if(frames[i] == -1)
      printf(1, "  -");
    else
      printf(1, "  %d", frames[i]);
  }
  printf(1, " ]");
}

// Simulate FIFO replacement
static int
run_fifo(int num_frames, int refs[], int nrefs, int verbose)
{
  int frames[MAX_FRAMES];
  int next_replace = 0;
  int faults = 0;

  for(int i = 0; i < num_frames; i++)
    frames[i] = -1;

  if(verbose)
    printf(1, "\n--- FIFO Simulation (%d frames, %d references) ---\n", num_frames, nrefs);

  for(int i = 0; i < nrefs; i++){
    int page = refs[i];
    int hit = 0;

    // Check if page is currently in any frame
    for(int j = 0; j < num_frames; j++){
      if(frames[j] == page){
        hit = 1;
        break;
      }
    }

    if(!hit){
      faults++;
      frames[next_replace] = page;
      next_replace = (next_replace + 1) % num_frames;
    }

    if(verbose){
      printf(1, "Step %d: Ref %d -> ", i + 1, page);
      print_frames(frames, num_frames);
      if(hit)
        printf(1, " [HIT]\n");
      else
        printf(1, " [FAULT]\n");
    }
  }

  int hits = nrefs - faults;
  if(verbose){
    printf(1, "FIFO Results: Page Faults = %d, Hits = %d, Hit Ratio = ", faults, hits);
    print_ratio(hits, nrefs);
    printf(1, "\n");
  }

  return faults;
}

// Simulate LRU replacement
static int
run_lru(int num_frames, int refs[], int nrefs, int verbose)
{
  int frames[MAX_FRAMES];
  int last_used[MAX_FRAMES];
  int faults = 0;

  for(int i = 0; i < num_frames; i++){
    frames[i] = -1;
    last_used[i] = -1;
  }

  if(verbose)
    printf(1, "\n--- LRU Simulation (%d frames, %d references) ---\n", num_frames, nrefs);

  for(int i = 0; i < nrefs; i++){
    int page = refs[i];
    int hit = 0;
    int hit_idx = -1;

    // Check if page is in frames
    for(int j = 0; j < num_frames; j++){
      if(frames[j] == page){
        hit = 1;
        hit_idx = j;
        break;
      }
    }

    if(hit){
      last_used[hit_idx] = i;
    } else {
      faults++;
      // Check for empty frame first
      int target_frame = -1;
      for(int j = 0; j < num_frames; j++){
        if(frames[j] == -1){
          target_frame = j;
          break;
        }
      }

      // If no empty frame, find least recently used
      if(target_frame == -1){
        int min_used = last_used[0];
        target_frame = 0;
        for(int j = 1; j < num_frames; j++){
          if(last_used[j] < min_used){
            min_used = last_used[j];
            target_frame = j;
          }
        }
      }

      frames[target_frame] = page;
      last_used[target_frame] = i;
    }

    if(verbose){
      printf(1, "Step %d: Ref %d -> ", i + 1, page);
      print_frames(frames, num_frames);
      if(hit)
        printf(1, " [HIT]\n");
      else
        printf(1, " [FAULT]\n");
    }
  }

  int hits = nrefs - faults;
  if(verbose){
    printf(1, "LRU Results: Page Faults = %d, Hits = %d, Hit Ratio = ", faults, hits);
    print_ratio(hits, nrefs);
    printf(1, "\n");
  }

  return faults;
}

int
main(int argc, char *argv[])
{
  // Default assignment reference string:
  // 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1
  int default_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
  int default_nrefs = sizeof(default_refs) / sizeof(default_refs[0]);

  int refs[MAX_REFS];
  int nrefs = 0;
  int custom_frames = 0;

  if(argc > 2){
    custom_frames = atoi(argv[1]);
    if(custom_frames <= 0 || custom_frames > MAX_FRAMES){
      printf(1, "Invalid frame count (1-%d)\n", MAX_FRAMES);
      exit();
    }
    for(int i = 2; i < argc && nrefs < MAX_REFS; i++){
      refs[nrefs++] = atoi(argv[i]);
    }
    run_fifo(custom_frames, refs, nrefs, 1);
    run_lru(custom_frames, refs, nrefs, 1);
    exit();
  }

  printf(1, "========================================================\n");
  printf(1, " Operating Systems Assignment 9 - Question 3: Page Repl \n");
  printf(1, " Reference string: 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1\n");
  printf(1, " Total references: %d\n", default_nrefs);
  printf(1, "========================================================\n");

  // Task 3b: Test with 3 frames, verbose tracing
  printf(1, "\n>>> Question 3(b): 3 Frames Detailed Execution <<<\n");
  int fifo_3 = run_fifo(3, default_refs, default_nrefs, 1);
  int lru_3 = run_lru(3, default_refs, default_nrefs, 1);

  printf(1, "\nVerification for 3 frames:\n");
  printf(1, "  FIFO expected: 15 faults | Actual: %d (%s)\n", fifo_3, fifo_3 == 15 ? "PASS" : "FAIL");
  printf(1, "  LRU  expected: 12 faults | Actual: %d (%s)\n", lru_3, lru_3 == 12 ? "PASS" : "FAIL");

  // Task 3c: Comparison table for 3, 4, 5 frames
  printf(1, "\n>>> Question 3(c): Comparison for 3, 4, and 5 Frames <<<\n");
  printf(1, "+--------+-------------------------+-------------------------+\n");
  printf(1, "| Frames |       FIFO Algorithm    |       LRU Algorithm     |\n");
  printf(1, "|        | Faults | Hit Ratio      | Faults | Hit Ratio      |\n");
  printf(1, "+--------+--------+----------------+--------+----------------+\n");

  int frame_tests[3] = {3, 4, 5};
  for(int k = 0; k < 3; k++){
    int f = frame_tests[k];
    int ff = run_fifo(f, default_refs, default_nrefs, 0);
    int fh = default_nrefs - ff;
    int lf = run_lru(f, default_refs, default_nrefs, 0);
    int lh = default_nrefs - lf;

    printf(1, "|   %d    |   %d   | ", f, ff);
    print_ratio(fh, default_nrefs);
    printf(1, "          |   %d    | ", lf);
    print_ratio(lh, default_nrefs);
    printf(1, "          |\n");
  }
  printf(1, "+--------+--------+----------------+--------+----------------+\n");
  printf(1, "\nAll Question 3 tasks completed successfully.\n");

  exit();
}
