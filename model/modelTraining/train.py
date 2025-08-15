# train.py
import pandas as pd
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
import joblib
from preprocess import generate_dns_data, feature_engineer

ATTACK_RATIO = 0.05

if __name__ == "__main__":
    # Generate and label data
    raw_logs = generate_dns_data(num_records=20000, attack_ratio=ATTACK_RATIO)
    raw_logs['is_attack_label'] = (raw_logs['label'] == 'attack').astype(int)
    
    true_labels_grouped = raw_logs.set_index('timestamp').groupby(
        ['source_ip', pd.Grouper(freq='10S')]
    )['is_attack_label'].max().reset_index()
    true_labels_grouped['true_label'] = true_labels_grouped['is_attack_label'].apply(
        lambda x: 'attack' if x == 1 else 'normal'
    )

    # Feature engineering
    features_df = feature_engineer(raw_logs, window_size='10S')
    features_df = pd.merge(
        features_df,
        true_labels_grouped[['source_ip', 'timestamp', 'true_label']],
        on=['source_ip', 'timestamp']
    )

    # Prepare training data
    model_features = [
        'query_count', 'unique_domains', 'unique_query_types',
        'error_rate', 'avg_latency', 'uniqueness_ratio'
    ]
    X = features_df[model_features].fillna(0)

    scaler = StandardScaler()
    X_scaled = scaler.fit_transform(X)

    model = IsolationForest(n_estimators=100, contamination=ATTACK_RATIO, random_state=42)
    model.fit(X_scaled)

    # Save model + scaler
    joblib.dump(model, "model.pkl")
    joblib.dump(scaler, "scaler.pkl")
    print("Model and scaler saved.")
