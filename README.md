# Autonomous Landslide & Flash-Flood Early Warning Grid (ALF-EWG)

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-14%2F17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Course](https://img.shields.io/badge/MAIT-CIC--211%20OOP-orange.svg)]()

A high-performance, object-oriented IoT telemetry ingestion and disaster management backend implemented in standard modern C++. Engineered for vulnerable mountainous terrains (such as the Himalayan belt), **ALF-EWG** autonomously ingests multi-modal sensor telemetry, computes hazard risk indexes in real time, triggers regional emergency sirens, evaluates priority-based evacuation corridors, and persistently logs incident histories to disk with transactional rollback safety.

---

## 📌 Academic Submission Details

- **Institution:** Maharaja Agrasen Institute of Technology (MAIT), Delhi
- **Department:** Department of Computer Science and Engineering
- **Subject:** Object-Oriented Programming Using C++
- **Paper Code:** CIC-211
- **Project Selection:** Option 1 (Autonomous Landslide & Flash-Flood Early Warning Grid)
- **Implementation Standard:** Clean ISO C++14/17 Standard
- **Submission Date:** November 2026

---

## 🏗️ System Architecture & OOP Design

### 1. Diamond Problem Resolution (Virtual Base Classes)
To model hybrid autonomous probes combining solar harvesting and LoRa wireless telemetry without duplicate subobject state ambiguity, **ALF-EWG** implements virtual inheritance:

```
               DisasterTelemetryDevice          <-- Virtual Base Class (Root)
               /                     \
    SolarPoweredNode             WirelessLoRaTransceiver   <-- Virtually Inherited
               \                     /
            AutonomousHybridProbe               <-- Leaf Node (Single Root Copy)
```

### 2. Containership (Aggregation) vs. Classification
Rather than relying on deep, brittle inheritance trees, `PowerSubsystem` (battery management, solar input calculation, power depletion) is modeled as an independent, encapsulated class and aggregated directly inside `BaseSensorNode`.

---

## 🚀 Key Technical Features by Syllabus Unit

### Unit I: Core Foundations & Call Semantics
- **Namespaces:** Code is partitioned into distinct namespaces (`DisasterCore::Sensors` and `DisasterCore::Alerts`) to prevent global namespace pollution.
- **Inline Functions:** Mathematical risk-indexing functions are marked `inline` for low-latency calculations during high-frequency telemetry polling.
- **Function Overloading:** Overloaded `computeHazardIndex()` handles varied telemetry combinations (rainfall + slope, soil saturation + vibration + river sonar, and full 4-parameter fusion).
- **Call & Return by Reference:** Geocoordinate transformations use call-by-reference (`double&`), while telemetry state accumulation uses return-by-reference to eliminate unnecessary data copies.
- **Friend Class:** `TelemetryAuditor` is declared a friend of `SensorStation`, inspecting private sensor buffers and coordinates without exposing internal getters.

### Unit II: Encapsulation, Dynamic Memory & Operator Overloading
- **Entities & Const Correctness:** `SensorStation`, `EarlyWarningSiren`, and `TelemetryBuffer` enforce strict data hiding and `const` member correctness.
- **Static Tracking Members:** Distributed network telemetry is tracked across all instances via static members (`SensorStation::activeCount_`, `globalEmergencyLevel_`, `EarlyWarningSiren::totalSirens_`).
- **Dynamic Memory Management:** `TelemetryBuffer` manages heap-allocated dynamic arrays with explicit parameterized constructors, deep copy constructors, copy assignment operators (`operator=`), and clean destructors (`delete[]`), eliminating memory leaks (RAII).
- **Operator Overloading:**
  - `operator+`: Combines sensor stations into regional zone vectors (`std::vector<SensorStation*>`).
  - `operator>`: Relational comparison prioritizing higher hazard values for triage.
  - `operator<<` & `operator>>`: Stream insertion for formatted tabular station reporting and stream extraction for reading raw telemetry streams.

### Unit III: Inheritance, Containership & Polymorphism
- **Abstract Base Class:** `BaseSensorNode` defines pure virtual interfaces (`readTelemetry() = 0` and `evaluateThreshold() = 0`).
- **Virtual Base Classes:** Resolves the diamond inheritance ambiguity for `AutonomousHybridProbe`.
- **Runtime Polymorphic Dispatch:** `RegionalSweepEngine` maintains a collection of base pointers (`std::vector<BaseSensorNode*>`), executing cyclical sweeps bound dynamically at runtime via `vtable` / `_vptr`.

### Unit IV: Streams, Persistence, Exception Safety & STL
- **Custom Exception Hierarchy:** `SensorFailureException`, `TelemetryChecksumMismatch`, and `ThresholdOverflowException` inherit from `std::exception` with custom `what()` diagnostic messages.
- **Binary Persistence & Transactional Rollbacks:** `IncidentLogger` writes POD structs using binary streams (`std::fstream`). Transactions utilize `seekp()` and `seekg()` to mark corrupt writes as rolled back without truncating files destructively.
- **STL Algorithms & Functors:** Utilizes `std::vector`, `std::map`, `std::sort`, `std::for_each`, and custom function objects (`HazardPriorityComparator`, `AlertDispatcher`) for priority-based evacuation routing.
- **Custom Stream Manipulators:** `alertHeader`, `alertFooter`, and `ColFmt` provide aligned tabular console reporting.

---

## 📂 Repository File Structure

```
├── ALF-EWG.cpp                            # Complete C++ implementation (Units I-IV)
├── ALF-EWG_Technical_Project_Report.pdf   # 5-Page formal technical project report
├── generate_report.py                     # ReportLab script used to build the PDF report
├── .gitignore                             # Ignores binaries, object files, logs, and caches
└── README.md                              # Project documentation and guide
```

---

## 🛠️ Build and Execution Guide

### Prerequisites
- Any C++14/C++17 compliant compiler (`g++`, `clang++`, or MSVC).
- Standard C++ Library.

### Compiling with GCC / MinGW:
```bash
g++ -std=c++14 -Wall -Wno-reorder -o ALF-EWG.exe ALF-EWG.cpp
```

### Running the Executable:
```bash
# On Windows:
./ALF-EWG.exe

# On Linux / macOS:
./ALF-EWG
```

---

## 📊 Sample Program Output

```text
========================================================================
  ALF-EWG: Autonomous Landslide & Flash-Flood Early Warning Grid
========================================================================

--- Booting Sensor Stations ---
  [Alert] Ingestion warning: ThresholdOverflowException: value=92.000000 exceeded threshold=90.000000

--- Activating Early Warning Sirens ---
  [SIREN #1 @ Kedarnath-Village] ACTIVATED -> RED (Danger)
  [SIREN #2 @ Badrinath-Gate] ACTIVATED -> YELLOW (Watch)
  [SIREN #3 @ Chamoli-Township] ACTIVATED -> CRITICAL (Evacuate)
  Network: 3 sirens total, 3 active | Stations online: 3

--- Regional Station Status ---
========================================================================
ID   Station               HazardIndex   AlertLevel            Active   
------------------------------------------------------------------------
1    Kedarnath-North       67.00         RED (Danger)          YES      
2    Badrinath-East        31.85         YELLOW (Watch)        YES      
3    Chamoli-Valley        92.00         CRITICAL (Evacuate)   YES      
========================================================================

--- Calibrating Autonomous Hybrid Probes (Diamond Hierarchy) ---
  [Device AHP-KDN-01] Calibrated at (30.7340, 79.0660)
  [Device AHP-BDR-02] Calibrated at (30.7440, 79.4930)
  [Device AHP-CHM-03] Calibrated at (30.4000, 79.3500)
  [Solar AHP-KDN-01] Harvested 10.80 Wh today

--- Cyclical Sensor Sweep (Polymorphic vtable Dispatch) ---
  Node[101] HazardIdx=51.90 -> ORANGE (Warning)
  Node[102] HazardIdx=37.10 -> YELLOW (Watch)
  [LoRa @867.00MHz SF=10.00] Pkt#1: ALERT! HazardIdx=61.440000
  Node[103] HazardIdx=61.44 -> RED (Danger)  ★ THRESHOLD TRIGGERED

--- Incident Logging (Binary fstream) ---
  [Logger] Written: Station#1 HazardIdx=67.0 Alert=RED (Danger)
  [Logger] Written: Station#2 HazardIdx=31.9 Alert=YELLOW (Watch)
  [Logger] COMMIT  at position 208
  [Logger] Written: Station#99 HazardIdx=999.9 Alert=CORRUPT
  Corrupt write detected -> rolling back...
  [Logger] ROLLBACK: marked 1 record(s) as rolled-back (seekp/seekg)
  Verified records on disk: 2
    Station#1  HazardIdx=67.0  Alert=RED (Danger)  Time=2026-11-11T15:00:00
    Station#2  HazardIdx=31.9  Alert=YELLOW (Watch)  Time=2026-11-11T15:05:00

--- Evacuation Route Dispatch ---
  [DISPATCH] Chamoli-Zone-C       Hazard=91.0    Pop=2500    Via: SH-13 via Gopeshwar
  [DISPATCH] Kedarnath-Zone-A     Hazard=85.0    Pop=1200    Via: NH-58 via Gaurikund
  [DISPATCH] Badrinath-Zone-B     Hazard=62.0    Pop=800     Via: NH-7 via Joshimath
  [DISPATCH] Rudraprayag-Zone-D   Hazard=45.0    Pop=600     Via: NH-58 via Tilwara
  Zones dispatched: 4 / 5

========================================================================
  ALF-EWG System shutdown complete. All heap memory released.
========================================================================
```

---

## 📜 License
This project is open-source under the MIT License.
