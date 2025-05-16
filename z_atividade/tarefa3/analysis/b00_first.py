#!/usr/bin/python
# %% Import analysis
import sys
sys.path.append(".")
from a00_parser import get_df

# %% Import auxiliary
import os
from matplotlib import pyplot as plt
import polars as pl

os.makedirs("./figures", exist_ok=True)

# %% Filtragem
df = get_df(os.getcwd())
df = df.filter(pl.col("action").ne("E"))
exps = {
    'A':'Sem dormir',
    'B':'Pai dorme',
    'C':'Filho dorme'
}
print(df.head())

# %% Figura 1
xticks = range(-10,120)

width_mm = 210 / 1
height_mm = 297 / 3

width_in = width_mm / 25.4
height_in = height_mm / 25.4

fig = plt.figure(figsize=(width_in,height_in))
for el,exp_letter in enumerate(exps.keys()):
    df_ag = df.filter(pl.col('exp_letter')==exp_letter)

    df_ag = df_ag.filter(pl.col("process_type") == "C")
    df_ag = df_ag.filter(pl.col("action").ne("E"))

    pid_delta = df_ag['pid']-df_ag['ppid']

    plt.subplot(3,3,el+1)
    plt.hist(pid_delta,bins=xticks)
    plt.xticks([xticks[0],xticks[-1]]);
    plt.xlabel(r"pid $\Delta$")
    if el == 0:
        plt.ylabel("Count")
    plt.title(exp_letter)
plt.savefig("./figures/delta_histogram.png", dpi=300, bbox_inches='tight', pad_inches=0)
plt.close(fig)


# %% Figura 2

xticks = range(-10,1500)

width_mm = 210 / 1
height_mm = 297 / 3

width_in = width_mm / 25.4
height_in = height_mm / 25.4

fig = plt.figure(figsize=(width_in,height_in))
for el,exp_letter in enumerate(exps.keys()):
    df_ag = df.filter(pl.col('exp_letter')==exp_letter)

    df_ag = df_ag.filter(pl.col("process_type") == "C")
    df_ag = df_ag.filter(pl.col("action").ne("E"))

    pid_delta = df_ag['pid']-df_ag['ppid']

    ax = plt.subplot(3,3,el+1)
    plt.plot(pid_delta)
    # plt.yticks = list(range(0,1000,250))
    plt.xlabel("Iteration #")
    if el == 0:
        plt.ylabel(r"pid $\Delta$")
    else:
        ax.set_yticklabels("")
    plt.title(exp_letter)
    plt.ylim([0,150])
    plt.grid()
plt.savefig("./figures/delta_iteration.png", dpi=300, bbox_inches='tight', pad_inches=0)
plt.close(fig)


# %% Figura 3
width_mm = 210 / 2
height_mm = 297 / 3

width_in = width_mm / 25.4
height_in = height_mm / 25.4

fig = plt.figure(figsize=(width_in,height_in))
count = [0 for _ in exps.keys()]
for el,exp_letter in enumerate(exps.keys()):
    df_ag = df.filter(pl.col('exp_letter')==exp_letter)
    df_on = df_ag.filter(pl.col('ppid')==1)
    count[el] = 100*len(df_on.rows())/len(df_ag.rows())

plt.bar(x=exps.keys(),height=count)
plt.title("Percentage of orphans")
plt.ylabel("Percentage")
plt.xlabel("Experiment Letter")
plt.yticks(range(0,26))
plt.grid()
plt.savefig("./figures/orphans.png", dpi=300, bbox_inches='tight', pad_inches=0)
plt.close(fig)
