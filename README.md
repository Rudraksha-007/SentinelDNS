# SentinelDNS

**A high-performance, authoritative DNS server written from scratch in C++ with integrated machine learning-based anomaly detection.
based on windows (winSock API) for networking**

SentinelDNS is designed for speed, security, and intelligent threat detection. The C++ core provides a lightweight, high-throughput DNS resolver, while the Python-based ML pipeline analyzes query patterns in real-time to identify and flag potential threats like DNS tunneling, DDoS amplification, and other anomalous activities.

## ✨ Features

*   **High-Performance Core:** Built in C++ for minimal latency and resource footprint.
*   **ML-Powered Anomaly Detection:** Utilizes an Isolation Forest model to detect unusual traffic patterns.
*   **Comprehensive Logging:** Captures detailed query-level and routing information for robust analysis.
*   **Flexible Deployment:** Run the C++ server and the ML inference engine independently or together.
*   **Easy to Train:** Includes scripts to train the anomaly detection model on your own data.

## ⚙️ How It Works

The system is composed of two main parts: the C++ DNS server and the Python ML pipeline.

### 1. DNS Server (C++)

The authoritative DNS server is implemented in [server/src/main.cpp](server/src/main.cpp). It's a lightweight UDP-based server that performs the following actions:
1.  Listens for incoming DNS queries on port 53.
2.  Parses the raw DNS query packets to extract the domain name, query type, and other header information.
3.  Looks up the requested domain in its local records.
4.  Constructs and sends a valid DNS response.
5.  Logs detailed information for every query to be consumed by the ML pipeline.

### 2. Anomaly Detection (Python)

The ML pipeline uses traffic logs to distinguish normal behavior from attacks.

*   **Feature Engineering:** Raw logs are aggregated into time windows (e.g., 10 seconds) to create meaningful features. This is handled by the `feature_engineer` function in [model/modelTraining/preprocess.py](model/modelTraining/preprocess.py).
*   **Model Training:** An Isolation Forest model is trained on the engineered features to learn what constitutes "normal" traffic. See the training script at [model/modelTraining/train.py](model/modelTraining/train.py).
*   **Inference:** The trained model and scaler are used to score new, incoming log data and predict whether it's an anomaly. The prediction logic is in [model/inference/predict.py](model/inference/predict.py).

## 📊 Logged Data Fields

The model's effectiveness relies on rich, detailed logs. The server is designed to capture the following fields for each query:

#### Query-Level Fields
*   **Timestamp:** UNIX or high-res milliseconds
*   **Source IP:** Client making the request
*   **Queried Domain Name:** The requested `example.com`
*   **Query Type:** `A`, `AAAA`, `MX`, `TXT`, etc.
*   **Protocol:** `UDP` or `TCP`
*   **Response Code:** `NOERROR`, `NXDOMAIN`, `SERVFAIL`, etc.
*   **Latency:** Milliseconds from request to response

#### Routing & Response Fields
*   **Destination IP:** The IP address returned in the response
*   **ASN or GeoIP:** Lookup data for the destination IP
*   **List Status:** Whether the destination IP is on a known "allow" or "block" list

## 🚀 Getting Started

### Prerequisites
*   A C++ compiler (g++, Clang, MSVC)
*   Python 3.x
*   Pip packages: `pandas`, `scikit-learn`, `joblib`, `numpy`

### 1. Train the Model
First, generate the model and scaler files.
```sh
python model/modelTraining/train.py
```
This will create `model.pkl` and `scaler.pkl` in the root directory.

### 2. Build and Run the Server
Compile the C++ server.
```sh
# For GCC/G++
g++ server/src/main.cpp -o main.exe -lws2_32

# For MSVC (in a Developer Command Prompt)
cl server/src/main.cpp /link /out:main.exe ws2_32.lib
```
Run the server. Note that binding to port 53 typically requires administrator privileges.
```sh
./main.exe
```

### 3. Perform Inference
You can test the prediction script on newly generated data.
```sh
python model/inference/predict.py
```

## License

This project is licensed under the MIT License.

---

Copyright (c) 2023 Sentinal-DNS

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
