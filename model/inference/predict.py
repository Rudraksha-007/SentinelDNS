# predict.py
import joblib
import pandas as pd
from feature import extract_features_from_logs

def predict_anomalies(features_df):
    model = joblib.load("model.pkl")
    scaler = joblib.load("scaler.pkl")

    model_features = [
        'query_count', 'unique_domains', 'unique_query_types',
        'error_rate', 'avg_latency', 'uniqueness_ratio'
    ]
    X = features_df[model_features].fillna(0)
    X_scaled = scaler.transform(X)

    predictions = model.predict(X_scaled)
    features_df['anomaly_score'] = model.decision_function(X_scaled)
    features_df['is_anomaly'] = predictions
    features_df['predicted_label'] = features_df['is_anomaly'].apply(lambda x: 'attack' if x == -1 else 'normal')
    return features_df

if __name__ == "__main__":
    # Example: load some logs from a file or generate them again
    from preprocess import generate_dns_data
    raw_logs = generate_dns_data(num_records=5000, attack_ratio=0.05)
    features_df = extract_features_from_logs(raw_logs)

    results = predict_anomalies(features_df)
    print(results.head())
