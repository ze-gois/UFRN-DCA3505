#!/usr/bin/python
import sys
sys.path.append(".")
from a00_parser import get_df
from matplotlib import pyplot
import polars as pl

df = get_df()

print(df.head())
