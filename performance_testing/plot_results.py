import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

test_dir_path = "performance_testing/test_results"
csv_paths = {
  "set_serial": f"{test_dir_path}/serial_results_set.csv",
  "set_parallel": f"{test_dir_path}/parallel_results_set.csv",
  "map_serial": f"{test_dir_path}/serial_results_map.csv",
  "map_parallel": f"{test_dir_path}/parallel_results_map.csv",
}

##############################
### HASH SETS
##############################

def plot_hash_set_serial():
  ################### 
  ## runtime
  ###################
  df = pd.read_csv(csv_paths['set_serial'])
  df = df[df['operation'] == 'insert']
  df['time'] = df['time'] * 1_000

  sns.lineplot(data=df, x='N', y='time', marker='o', hue='set_impl')
  plt.legend(title='Set Implementation')
  # plt.title('Hash Set test: Inserting Integers 1...N')
  plt.xlabel('N (thousands)')
  plt.ylabel('Time (ms)')
  plt.savefig(f'{test_dir_path}/set_serial_insert_time.png', dpi=300)
  plt.close()

  ###################
  ## performance
  ###################
  df = df[df['operation'] == 'insert']
  df['performance'] =  df['N'] / df['time'] / 1_000_000
  df['N'] = df['N'] / 1_000
  sns.lineplot(data=df, x='N', y='performance', marker='o', hue='set_impl')
  plt.legend(title='Set Implementation')
  plt.title('Hash Set test: Inserting Integers 1...N')
  plt.xlabel('N (thousands)')
  plt.ylabel('Performance (insertions/ns)')
  plt.savefig(f'{test_dir_path}/set_serial_insert_perf.png', dpi=300)
  plt.close()


def plot_hash_set_speedup():
  ################### 
  ## speedup on 200_000
  ###################
  df = pd.read_csv(csv_paths['set_parallel'])
  df = df[df['operation'] == 'insert']
  df = df[df['N'] == 200_000]
  
  df_p1 = df[df['P'] == 1].copy()
  df_p1 = df_p1.rename(columns={'time': 'time_p1'})
  df_p1 = df_p1[['N', 'set_impl', 'operation', 'time_p1']]
  df_merged = pd.merge(df, df_p1, on=['N', 'set_impl', 'operation'])
  df_merged['normalized_time'] = df_merged['time'] / df_merged['time_p1']
  df_speedup = df_merged.drop(columns=['time_p1'])

  sns.barplot(data=df_speedup, x='set_impl', y='normalized_time', hue='P')
  plt.title('Hash Set test: Slow down on 200K insertions')
  plt.xlabel('Set Implementation')
  plt.ylabel('P threads runtime / 1 thread runtime')
  plt.legend(title='Number of threads')
  plt.savefig(f'{test_dir_path}/set_parallel_insert_speedup.png', dpi=300)
  plt.close()

  ###################
  ## runtime with 16 threads
  ###################
  df = pd.read_csv(csv_paths['set_parallel'])
  df = df[(df['operation'] == 'insert') * (df['P'] == 16)]
  df['performance'] =  df['N'] / df['time'] / 1_000_000
  df['N'] = df['N'] / 1_000

  sns.lineplot(data=df, x='N', y='performance', marker='o', hue='set_impl')
  plt.legend(title='Set Implementation')
  plt.title('Hash Set test: Inserting Integers 1...N with 16 threads')
  plt.xlabel('N (thousands)')
  plt.ylabel('Performance (insertions/μs)')
  plt.savefig(f'{test_dir_path}/set_parallel16_insert_perf.png', dpi=300)
  plt.close()


##############################
### HASH MAPS
##############################

def plot_hash_map_serial():
  ################### 
  ## runtime
  ###################
  df = pd.read_csv(csv_paths['map_serial'])
  df['time'] = df['time']
  df['operation'] = df['operation'].replace('map_clear', 'clear')


  sns.barplot(data=df, x='map_impl', y='time', hue='operation')
  plt.legend(title='Algorithm Stage')
  plt.title('Semi-Pattern Join of 400K strings with 1 thread')
  plt.xlabel('Map Implementation')
  plt.ylabel('Time (s)')
  plt.savefig(f'{test_dir_path}/map_serial_simsearch_time.png', dpi=300)
  plt.close()

def plot_hash_map_speedup():
  ################### 
  ## speedup
  ###################
  df = pd.read_csv(csv_paths['map_parallel'])
  df['map_impl'] = df['map_impl'].replace('ankerl_mapreduce', 'ankerl, map-reduce')
  df['map_impl'] = df['map_impl'].replace('gtl_mapreduce2', 'gtl, reduce')
  df['map_impl'] = df['map_impl'].replace('gtl', 'gtl<tbb_vector>')
  df['total_time'] = df.groupby(['N', 'map_impl', 'P'])['time'].transform('sum')
  df_total_time = df[['N', 'map_impl', 'P', 'total_time']].drop_duplicates()
  df_total_time['operation'] = 'total_time'
  df_total_time = df_total_time.rename(columns={'total_time': 'time'})
  
  df_p1 = df_total_time[df_total_time['operation'] == 'total_time']
  df_p1 = df_p1[df_p1['P'] == 1]
  df_p1 = df_p1.rename(columns={'time': 'time_p1'})
  df_p1 = df_p1[['N', 'map_impl', 'operation', 'time_p1']]
  df_merged = pd.merge(df_total_time, df_p1, on=['N', 'map_impl', 'operation'])
  df_merged['normalized_time'] = df_merged['time_p1'] / df_merged['time']
  df_speedup = df_merged.drop(columns=['time_p1'])
  sns.lineplot(data=df_speedup, x='P', y='normalized_time', marker='o', hue='map_impl')
  plt.title('Speedup of Semi-Pattern Join')
  plt.legend(title='Parallelization Strategy')
  plt.xlabel('Number of threads')
  plt.ylabel('1 thread runtime / P threads runtime')
  plt.savefig(f'{test_dir_path}/map_parallel_simsearch_speedup.png', dpi=300)
  plt.close()

  ################### 
  ## runtime on 16 threads
  ###################
  sns.barplot(data=df, x='P', y='time', hue='map_impl', errorbar=None)
  plt.legend(title='Parallelization Strategy')
  plt.title('Semi-Pattern Join Runtime with different number of threads')
  plt.xlabel('Number of threads')
  plt.ylabel('Runtime (s)')
  plt.savefig(f'{test_dir_path}/map_parallel_runtime.png', dpi=300)
  plt.close()

  ###################
  ## clearing time
  ###################
  df_clear = df[df['operation'] == 'map_clear']
  df_clear = pd.merge(df_clear, df_total_time, on=['N', 'map_impl', 'P'])
  df_clear['clear_part'] = df_clear['time_x'] / df_clear['time_y'] * 100
  df_clear = df_clear[['map_impl', 'P', 'clear_part']]
  sns.barplot(data=df_clear, x='P', y='clear_part', hue='map_impl')
  plt.title('Part taken by clearing the map in Semi-Pattern Join')
  plt.legend(title='Parallelization Strategy')
  plt.xlabel('Clear part of the algorithm (%)')
  plt.ylabel('Runtime (s)')
  plt.savefig(f'{test_dir_path}/map_parallel_clear_part.png', dpi=300)
  plt.close()

def final_algo_speedup():
  thread_num = [1, 2, 4, 8, 16]
  runtime_withz = [43.53, 29.71, 19.71, 14.18, 15.39]
  runtime_noz = [43.00, 27.05, 20.33, 17.21, 18.35]
  speedup_withz = [runtime_withz[0] / r for r in runtime_withz]
  speedup_noz = [runtime_noz[0] / r for r in runtime_noz]
  data = {
    'Number of threads': thread_num * 2,
    'Speedup': speedup_withz + speedup_noz,
    'Imbalance parameter': ['20K'] * len(thread_num) + ['∞'] * len(thread_num)
  }
  df = pd.DataFrame(data)
  sns.lineplot(data=df, x='Number of threads', y='Speedup', marker='o', hue='Imbalance parameter')
  plt.title('Speedup of Partition-Pattern Join algorithm')
  plt.xlabel('Number of threads')
  plt.ylabel('Speedup')
  plt.savefig(f'{test_dir_path}/final_algo_speedup.png', dpi=300)
  plt.close()

if __name__ == "__main__":
  plot_hash_set_serial()
  plot_hash_set_speedup()
  plot_hash_map_serial()
  plot_hash_map_speedup()
  final_algo_speedup()