# ManualDiskCleaning

> **⚠️ Project Status: Beta**
> Please note that this project is currently in its Beta stage. Some things might not work.

## Overview
**ManualDiskCleaning** is a high-performance Windows file system analyzer designed to bypass the slow native OS directory loading times. 

Traditionally, checking the size of large directories (like `Program Files`) via Windows Properties can take a significant amount of time. This project solves that bottleneck by mapping an entire drive in under 1 minute (depending on the drive and cpu) and providing an instant, responsive overview of your disk space consumption.

## Architecture
The application utilizes a decoupled architecture to separate heavy computation from the user interface:
* **Backend Engine (C++)**: Handles all the heavy lifting to deliver maximum performance and eliminate native OS lags.
* **Frontend GUI (Electron JS)**: Communicates with the C++ engine via a custom API to deliver a responsive graphical interface for instant space analysis.

## Key Features
* **Lightning-Fast Scanning**: Maps entire hard drives in minutes rather than forcing you to wait for individual directory size calculations.
* **Instant Navigation**: Browse through your heaviest folders with zero loading times once the initial scan is complete.
* **Safe Deletion**: Files deleted through the application are safely moved to the Windows Recycle Bin, preventing accidental permanent data loss.

## UI

<table align="center">
  <tr>
    <td valign="top" style="padding: 5px; border: none;">
      <img src="https://github.com/user-attachments/assets/0cc8eb54-291b-4c03-be0f-0e7a727724a6" alt="image 1" width="230" />
    </td>
    <td valign="top" style="padding: 5px; border: none;">
      <img src="https://github.com/user-attachments/assets/a061c5c2-9031-443c-ae73-42e582b4c0b2" alt="image 2" width="230" />
    </td>
    <td valign="top" style="padding: 5px; border: none;">
      <img src="https://github.com/user-attachments/assets/0b88234a-76b7-419b-a54c-e4bd9fbe89e7" alt="image 3" width="230" />
    </td>
    <td valign="top" style="padding: 5px; border: none;">
      <img src="https://github.com/user-attachments/assets/5f5ff1e6-ada7-4668-b58d-2f0b83c9020a" alt="image 4" width="230" />
    </td>
  </tr>
</table>




## Getting Started

### Prerequisites & Installation
Currently, the application requires you to run the pre-compiled executables directly. No complex installation is required.

1. Navigate to the [Releases](../../releases) page of this repository.
2. Download the latest Alpha release containing both the Backend and Frontend executables.

### Usage Instructions
To ensure proper communication between the GUI and the engine, you must launch the executables in the following order:

1. **Start the Backend**: Run the backend `.exe` file first to initialize the C++ engine.
2. **Start the Frontend**: Once the backend is running, launch the frontend `.exe` to open the Electron GUI.
3. Start analyzing and cleaning up your disk space instantly!
