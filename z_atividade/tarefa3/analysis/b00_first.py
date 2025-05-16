#!/usr/bin/python

# %% Cell 1
import sys
sys.path.append(".")
from a00_parser import get_df
from matplotlib import pyplot
import polars as pl
import pandas as pd
df = get_df()

print(df.head())

# %%cell
all([a[0]==b for a,b in zip(*df[:,['exp_id','exp_letter']])])
all([a == (b+str(c)) for a,b,c in zip(*df[:,['exp_id','exp_letter','repetition']])])

# %%cell
