#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_REF 20
#define MAX_FRAMES 10

static int ref_string[MAX_REF] = {
  7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1
};
static int ref_len = 20;

struct sim_result {
  int faults;
  int hits;
};

// Helper function to print percentages without floating point support
static void
print_ratio(int num, int den)
{
  int pct_x100 = (num * 10000 + den / 2) / den;
  int whole = pct_x100 / 100;
  int frac = pct_x100 % 100;
  if(frac < 10)
    printf(1, "%d.0%d%%", whole, frac);
  else
    printf(1, "%d.%d%%", whole, frac);
}

// Helper to print current frame contents
static void
print_frames(int *frames, int num_frames)
{
  int i;
  printf(1, "[ ");
  for(i = 0; i < num_frames; i++){
    if(frames[i] == -1)
      printf(1, "- ");
    else
      printf(1, "%d ", frames[i]);
  }
  printf(1, "]");
}

// FIFO Page Replacement
static struct sim_result
run_fifo(int num_frames, int verbose)
{
  int frames[MAX_FRAMES];
  int i, j;
  int faults = 0, hits = 0;
  int victim_ptr = 0;

  for(i = 0; i < num_frames; i++)
    frames[i] = -1;

  if(verbose){
    printf(1, "\n--- FIFO Simulation (%d Frames) ---\n", num_frames);
    printf(1, "Step | Ref | Frames      | Status\n");
    printf(1, "-----+-----+-------------+---------\n");
  }

  for(i = 0; i < ref_len; i++){
    int page = ref_string[i];
    int hit = 0;

    for(j = 0; j < num_frames; j++){
      if(frames[j] == page){
        hit = 1;
        break;
      }
    }

    if(hit){
      hits++;
    } else {
      faults++;
      frames[victim_ptr] = page;
      victim_ptr = (victim_ptr + 1) % num_frames;
    }

    if(verbose){
      if(i + 1 < 10)
        printf(1, "  %d  |  %d  | ", i + 1, page);
      else
        printf(1, " %d  |  %d  | ", i + 1, page);
      print_frames(frames, num_frames);
      if(hit)
        printf(1, " | Page Hit\n");
      else
        printf(1, " | Page Fault\n");
    }
  }

  if(verbose){
    printf(1, "-----------------------------------\n");
    printf(1, "FIFO Summary (%d frames): Page Faults = %d, Page Hits = %d\n\n",
           num_frames, faults, hits);
  }

  struct sim_result res;
  res.faults = faults;
  res.hits = hits;
  return res;
}

// LRU Page Replacement
static struct sim_result
run_lru(int num_frames, int verbose)
{
  int frames[MAX_FRAMES];
  int last_used[MAX_FRAMES];
  int i, j;
  int faults = 0, hits = 0;

  for(i = 0; i < num_frames; i++){
    frames[i] = -1;
    last_used[i] = -1;
  }

  if(verbose){
    printf(1, "\n--- LRU Simulation (%d Frames) ---\n", num_frames);
    printf(1, "Step | Ref | Frames      | Status\n");
    printf(1, "-----+-----+-------------+---------\n");
  }

  for(i = 0; i < ref_len; i++){
    int page = ref_string[i];
    int hit = 0;
    int hit_idx = -1;

    for(j = 0; j < num_frames; j++){
      if(frames[j] == page){
        hit = 1;
        hit_idx = j;
        break;
      }
    }

    if(hit){
      hits++;
      last_used[hit_idx] = i;
    } else {
      faults++;
      // Check for empty frame first
      int empty_idx = -1;
      for(j = 0; j < num_frames; j++){
        if(frames[j] == -1){
          empty_idx = j;
          break;
        }
      }

      if(empty_idx != -1){
        frames[empty_idx] = page;
        last_used[empty_idx] = i;
      } else {
        // Find frame with minimum last_used timestamp
        int lru_idx = 0;
        int min_time = last_used[0];
        for(j = 1; j < num_frames; j++){
          if(last_used[j] < min_time){
            min_time = last_used[j];
            lru_idx = j;
          }
        }
        frames[lru_idx] = page;
        last_used[lru_idx] = i;
      }
    }

    if(verbose){
      if(i + 1 < 10)
        printf(1, "  %d  |  %d  | ", i + 1, page);
      else
        printf(1, " %d  |  %d  | ", i + 1, page);
      print_frames(frames, num_frames);
      if(hit)
        printf(1, " | Page Hit\n");
      else
        printf(1, " | Page Fault\n");
    }
  }

  if(verbose){
    printf(1, "-----------------------------------\n");
    printf(1, "LRU Summary (%d frames): Page Faults = %d, Page Hits = %d\n\n",
           num_frames, faults, hits);
  }

  struct sim_result res;
  res.faults = faults;
  res.hits = hits;
  return res;
}

// Optimal Page Replacement
static struct sim_result
run_opt(int num_frames, int verbose)
{
  int frames[MAX_FRAMES];
  int i, j, k;
  int faults = 0, hits = 0;

  for(i = 0; i < num_frames; i++)
    frames[i] = -1;

  if(verbose){
    printf(1, "\n--- Optimal Simulation (%d Frames) ---\n", num_frames);
    printf(1, "Step | Ref | Frames      | Status\n");
    printf(1, "-----+-----+-------------+---------\n");
  }

  for(i = 0; i < ref_len; i++){
    int page = ref_string[i];
    int hit = 0;

    for(j = 0; j < num_frames; j++){
      if(frames[j] == page){
        hit = 1;
        break;
      }
    }

    if(hit){
      hits++;
    } else {
      faults++;
      // Check for empty frame
      int empty_idx = -1;
      for(j = 0; j < num_frames; j++){
        if(frames[j] == -1){
          empty_idx = j;
          break;
        }
      }

      if(empty_idx != -1){
        frames[empty_idx] = page;
      } else {
        // Find page whose next occurrence is furthest in future
        int victim_idx = -1;
        int furthest = -1;

        for(j = 0; j < num_frames; j++){
          int next_use = -1;
          for(k = i + 1; k < ref_len; k++){
            if(ref_string[k] == frames[j]){
              next_use = k;
              break;
            }
          }

          if(next_use == -1){
            // Not used again in future: ideal victim
            victim_idx = j;
            break;
          }

          if(next_use > furthest){
            furthest = next_use;
            victim_idx = j;
          }
        }

        frames[victim_idx] = page;
      }
    }

    if(verbose){
      if(i + 1 < 10)
        printf(1, "  %d  |  %d  | ", i + 1, page);
      else
        printf(1, " %d  |  %d  | ", i + 1, page);
      print_frames(frames, num_frames);
      if(hit)
        printf(1, " | Page Hit\n");
      else
        printf(1, " | Page Fault\n");
    }
  }

  if(verbose){
    printf(1, "-----------------------------------\n");
    printf(1, "Optimal Summary (%d frames): Page Faults = %d, Page Hits = %d\n\n",
           num_frames, faults, hits);
  }

  struct sim_result res;
  res.faults = faults;
  res.hits = hits;
  return res;
}

int
main(int argc, char *argv[])
{
  printf(1, "\n=================================================================\n");
  printf(1, "  Question 4: Page Replacement Algorithms (FIFO, LRU, Optimal)  \n");
  printf(1, "=================================================================\n\n");

  printf(1, "Reference String (20 references):\n");
  printf(1, "  7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1\n\n");

  // Tasks a & b: Detailed execution for 3 frames
  printf(1, "*****************************************************************\n");
  printf(1, "  Part (a) & (b): Detailed Traces with 3 Frames                  \n");
  printf(1, "*****************************************************************\n");

  struct sim_result fifo3 = run_fifo(3, 1);
  struct sim_result lru3 = run_lru(3, 1);
  struct sim_result opt3 = run_opt(3, 1);

  printf(1, "Summary for 3 Frames:\n");
  printf(1, "  FIFO    Total Page Faults: %d\n", fifo3.faults);
  printf(1, "  LRU     Total Page Faults: %d\n", lru3.faults);
  printf(1, "  Optimal Total Page Faults: %d\n\n", opt3.faults);

  // Task c: Run with 3, 4, and 5 frames and display comparison table
  printf(1, "*****************************************************************\n");
  printf(1, "  Part (c): Performance Comparison Across 3, 4, and 5 Frames     \n");
  printf(1, "*****************************************************************\n\n");

  struct sim_result fifo4 = run_fifo(4, 0);
  struct sim_result lru4 = run_lru(4, 0);
  struct sim_result opt4 = run_opt(4, 0);

  struct sim_result fifo5 = run_fifo(5, 0);
  struct sim_result lru5 = run_lru(5, 0);
  struct sim_result opt5 = run_opt(5, 0);

  printf(1, "+--------+----------+-------------+-----------+-----------+------------------+\n");
  printf(1, "| Frames | Policy   | Page Faults | Page Hits | Hit Ratio | Page Fault Ratio |\n");
  printf(1, "+--------+----------+-------------+-----------+-----------+------------------+\n");

  // 3 Frames
  printf(1, "|   3    | FIFO     |     %d      |     %d     |  ", fifo3.faults, fifo3.hits);
  print_ratio(fifo3.hits, ref_len);
  printf(1, "   |      ");
  print_ratio(fifo3.faults, ref_len);
  printf(1, "      |\n");

  printf(1, "|   3    | LRU      |     %d      |     %d     |  ", lru3.faults, lru3.hits);
  print_ratio(lru3.hits, ref_len);
  printf(1, "   |      ");
  print_ratio(lru3.faults, ref_len);
  printf(1, "      |\n");

  printf(1, "|   3    | Optimal  |      %d      |    %d     |  ", opt3.faults, opt3.hits);
  print_ratio(opt3.hits, ref_len);
  printf(1, "   |      ");
  print_ratio(opt3.faults, ref_len);
  printf(1, "      |\n");

  printf(1, "+--------+----------+-------------+-----------+-----------+------------------+\n");

  // 4 Frames
  printf(1, "|   4    | FIFO     |     %d      |    %d     |  ", fifo4.faults, fifo4.hits);
  print_ratio(fifo4.hits, ref_len);
  printf(1, "   |      ");
  print_ratio(fifo4.faults, ref_len);
  printf(1, "      |\n");

  printf(1, "|   4    | LRU      |      %d      |    %d     |  ", lru4.faults, lru4.hits);
  print_ratio(lru4.hits, ref_len);
  printf(1, "   |      ");
  print_ratio(lru4.faults, ref_len);
  printf(1, "      |\n");

  printf(1, "|   4    | Optimal  |      %d      |    %d     |  ", opt4.faults, opt4.hits);
  print_ratio(opt4.hits, ref_len);
  printf(1, "   |      ");
  print_ratio(opt4.faults, ref_len);
  printf(1, "      |\n");

  printf(1, "+--------+----------+-------------+-----------+-----------+------------------+\n");

  // 5 Frames
  printf(1, "|   5    | FIFO     |      %d      |    %d     |  ", fifo5.faults, fifo5.hits);
  print_ratio(fifo5.hits, ref_len);
  printf(1, "   |      ");
  print_ratio(fifo5.faults, ref_len);
  printf(1, "      |\n");

  printf(1, "|   5    | LRU      |      %d      |    %d     |  ", lru5.faults, lru5.hits);
  print_ratio(lru5.hits, ref_len);
  printf(1, "   |      ");
  print_ratio(lru5.faults, ref_len);
  printf(1, "      |\n");

  printf(1, "|   5    | Optimal  |      %d      |    %d     |  ", opt5.faults, opt5.hits);
  print_ratio(opt5.hits, ref_len);
  printf(1, "   |      ");
  print_ratio(opt5.faults, ref_len);
  printf(1, "      |\n");

  printf(1, "+--------+----------+-------------+-----------+-----------+------------------+\n\n");

  printf(1, "Analysis & Observations:\n");
  printf(1, "  1. Optimal Page Replacement performs BEST overall across all frame sizes,\n");
  printf(1, "     producing the lowest fault counts: 9 (at 3 frames), 8 (at 4 frames),\n");
  printf(1, "     and 7 (at 5 frames). Because Optimal has clairvoyant future knowledge,\n");
  printf(1, "     it represents the theoretical lower bound on page faults.\n");
  printf(1, "  2. LRU performs significantly better than FIFO across all frame sizes (12 vs 15\n");
  printf(1, "     at 3 frames, 8 vs 10 at 4 frames, and 7 vs 9 at 5 frames). By exploiting\n");
  printf(1, "     temporal locality, LRU accurately predicts future page re-use from recent\n");
  printf(1, "     history, matching Optimal at 4 and 5 frames.\n");
  printf(1, "  3. FIFO exhibits the highest page fault rate because it evicts pages based\n");
  printf(1, "     strictly on arrival time, frequently evicting heavily accessed pages.\n\n");

  printf(1, "[pagereplace] Question 4 completed successfully!\n\n");
  exit();
}
