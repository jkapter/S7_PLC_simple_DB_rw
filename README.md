# Whatsminer Industrial Hardware Monitoring Bot

A robust C++ command-line utility and Telegram bot designed for real-time monitoring and diagnostics of **Whatsminer ASIC hardware**. This tool allows industrial mining farm operators to check device health, temperatures, and hashrates remotely.

## 🚀 Key Features
* **Hardware Diagnostics:** Remote monitoring of critical hardware metrics (Hashrate, Chip Temperature, Fan Speeds, Power Consumption).
* **Instant Alerts:** Built to help operators catch hardware failures, overheating, or hashing drops before they cause significant downtime.
* **Secure Remote Control:** Allows safe status polling via encrypted Telegram Bot API channels.

## 🛠️ Tech Stack & Requirements
* **Language:** C++
* **API Integration:** Telegram Bot API & Whatsminer API/Protocol
* **Build System:** CMake
* Qt6

## 🔧 Installation & Setup
1. Clone the repository to your local monitoring server or gateway.
2. Configure your specific hardware IP addresses and your Telegram Bot Token in the configuration files.
3. Build the project using CMake and run the executable.
