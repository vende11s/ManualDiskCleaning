<div align="center">
  <h1>Get Space</h1>
  <img width="256" height="256" alt="Get Space Logo" src="https://github.com/user-attachments/assets/a7e833bd-6e10-4f23-b45f-36c6145f4216" />
  <p><b>A high-performance Windows file system analyzer built for speed.</b></p>
</div>


## 🚀 Overview
Traditionally, checking the size of large directories (like `Program Files`) via Windows Properties takes ages. And when you finally open it to see *what* exactly is taking up so much space? You wait again. And again. 

**Get Space** solves this bottleneck. It maps your entire drive in seconds (utilizing advanced multithreading) and provides an instant, responsive overview of your disk space consumption. No more loading bars just to check folder sizes.

## 🏗️ Architecture
The application utilizes a decoupled architecture to separate heavy computation from the user interface:
* **Backend Engine (C++)**: A highly optimized multithreaded scanner that bypasses native OS directory loading times. It utilizes **thread-local storage** and an **ultra-low contention** task queue to handle heavy lifting and maximize CPU throughput.
* **Frontend GUI (Electron.js)**: Communicates with the C++ engine via a local HTTP API to deliver a beautiful, modern and responsive graphical interface.

## ✨ Key Features
* **Lightning-Fast Scanning**: Maps entire hard drives in seconds using advanced C++ Thread Pooling with minimal lock contention.
* **Zero-Lag Navigation**: Browse through your heaviest folders instantly once the initial scan is complete.
* **Native Recycle Bin Integration**: Select any file or folder and hit `Delete` to move it straight to the Recycle Bin, exactly like you would in native Windows Explorer.

## 📸 Quick look at UI
<img width="999" height="651" alt="Get Space UI" src="https://github.com/user-attachments/assets/cfec9eea-9b3d-433c-a755-d2bc6b4dbbbb" />

## 🏁 Getting Started

### Prerequisites & Installation
1. Navigate to the [Releases](../../releases) page of this repository.
2. Download the latest release installer (`.exe`).
3. Install, run, and instantly find out what's eating your space!
