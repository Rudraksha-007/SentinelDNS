# feature.py
import pandas as pd
from preprocess import feature_engineer

def extract_features_from_logs(raw_logs, window_size='10S'):
    return feature_engineer(raw_logs, window_size)
