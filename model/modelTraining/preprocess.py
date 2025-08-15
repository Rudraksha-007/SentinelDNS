# preprocess.py
import pandas as pd
import numpy as np

def generate_dns_data(num_records=10000, attack_ratio=0.05):
    ips = [f"192.168.1.{np.random.randint(1, 255)}" for _ in range(50)]
    attack_ips = [f"10.0.0.{np.random.randint(1, 255)}" for _ in range(200)]
    domains = [f"service{i}.com" for i in range(10)]
    attack_domain = "victim-service.com"

    data = []
    for i in range(num_records):
        is_attack = np.random.rand() < attack_ratio
        if is_attack:
            record = {
                'timestamp': pd.to_datetime('now', utc=True).timestamp() + i * 0.001,
                'source_ip': np.random.choice(attack_ips),
                'domain': attack_domain,
                'query_type': 'A',
                'protocol': 'UDP',
                'response_code': 'NOERROR',
                'latency_ms': np.random.uniform(50, 150),
                'label': 'attack'
            }
        else:
            record = {
                'timestamp': pd.to_datetime('now', utc=True).timestamp() + i * 0.01,
                'source_ip': np.random.choice(ips),
                'domain': np.random.choice(domains),
                'query_type': np.random.choice(['A', 'AAAA', 'MX', 'TXT']),
                'protocol': 'UDP',
                'response_code': np.random.choice(['NOERROR', 'NXDOMAIN', 'SERVFAIL'], p=[0.9, 0.07, 0.03]),
                'latency_ms': np.random.uniform(20, 80),
                'label': 'normal'
            }
        data.append(record)

    df = pd.DataFrame(data)
    df['timestamp'] = pd.to_datetime(df['timestamp'], unit='s')
    return df

def feature_engineer(df, window_size='10S'):
    df = df.set_index('timestamp')
    grouped = df.groupby(['source_ip', pd.Grouper(freq=window_size)])
    features = grouped.agg(
        query_count=('domain', 'count'),
        unique_domains=('domain', 'nunique'),
        unique_query_types=('query_type', 'nunique'),
        error_rate=('response_code', lambda x: (x != 'NOERROR').sum() / len(x)),
        avg_latency=('latency_ms', 'mean')
    ).reset_index()
    features['uniqueness_ratio'] = features['unique_domains'] / features['query_count']
    return features
