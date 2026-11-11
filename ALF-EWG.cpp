// =============================================================================
// ALF-EWG: Autonomous Landslide & Flash-Flood Early Warning Grid
// CIC-211 OOP with C++ — Option 1 (Complete Implementation)
// C++17 Compliant | Covers all 4 syllabus units
// =============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <functional>
#include <ctime>

// =============================================================================
// UNIT I — Core Foundations & Call Semantics
//   • Namespaces: DisasterCore::Sensors, DisasterCore::Alerts
//   • Inline mathematical risk-indexing (overloaded computeHazardIndex)
//   • Call by reference & return by reference
//   • Friend class: TelemetryAuditor
// =============================================================================

namespace DisasterCore {

    namespace Alerts {
        enum class Level { GREEN = 0, YELLOW = 1, ORANGE = 2, RED = 3, CRITICAL = 4 };

        inline std::string levelToString(Level lv) {
            switch (lv) {
                case Level::GREEN:    return "GREEN (Safe)";
                case Level::YELLOW:   return "YELLOW (Watch)";
                case Level::ORANGE:   return "ORANGE (Warning)";
                case Level::RED:      return "RED (Danger)";
                case Level::CRITICAL: return "CRITICAL (Evacuate)";
                default:              return "UNKNOWN";
            }
        }

        inline Level hazardToLevel(double idx) {
            if (idx < 20.0) return Level::GREEN;
            if (idx < 40.0) return Level::YELLOW;
            if (idx < 60.0) return Level::ORANGE;
            if (idx < 80.0) return Level::RED;
            return Level::CRITICAL;
        }
    } // namespace Alerts

    namespace Sensors {

        // ── Overload 1: rainfall rate + slope angle ──────────────────────────
        inline double computeHazardIndex(double rainfallRate_mm_h, double slopeAngle_deg) {
            return (rainfallRate_mm_h * 0.60) + (slopeAngle_deg * 0.40);
        }

        // ── Overload 2: soil saturation + vibration frequency + river level ──
        inline double computeHazardIndex(double soilSaturation_pct,
                                          double vibrationFreq_Hz,
                                          double riverLevel_m) {
            return (soilSaturation_pct  * 0.40)
                 + (vibrationFreq_Hz    * 0.35)
                 + (riverLevel_m        * 0.25);
        }

        // ── Overload 3: all four combined parameters ──────────────────────────
        inline double computeHazardIndex(double rainfall, double slope,
                                          double saturation, double vibration) {
            return (rainfall    * 0.30)
                 + (slope       * 0.30)
                 + (saturation  * 0.20)
                 + (vibration   * 0.20);
        }

        // Call by reference — high-frequency geocoordinate transformation
        void transformGeoCoordinates(double& lat, double& lon,
                                     double dLat, double dLon) {
            lat += dLat;
            lon += dLon;
        }

        // Return by reference — telemetry state accumulation
        double& accumulateTelemetry(double& accumulator, double reading) {
            accumulator += reading;
            return accumulator;
        }

    } // namespace Sensors
} // namespace DisasterCore

// =============================================================================
// UNIT IV — Custom Exception Classes (declared early; used throughout)
// =============================================================================

class SensorFailureException : public std::exception {
    int         sensorId_;
    std::string msg_;
public:
    SensorFailureException(int id, const std::string& detail)
        : sensorId_(id),
          msg_("SensorFailureException [ID:" + std::to_string(id) + "]: " + detail) {}
    const char* what() const noexcept override { return msg_.c_str(); }
    int getSensorId() const { return sensorId_; }
};

class TelemetryChecksumMismatch : public std::exception {
    std::string msg_;
public:
    TelemetryChecksumMismatch(uint32_t expected, uint32_t received)
        : msg_("TelemetryChecksumMismatch: expected="
               + std::to_string(expected) + " got=" + std::to_string(received)) {}
    const char* what() const noexcept override { return msg_.c_str(); }
};

class ThresholdOverflowException : public std::exception {
    double value_, threshold_;
    std::string msg_;
public:
    ThresholdOverflowException(double val, double thresh)
        : value_(val), threshold_(thresh),
          msg_("ThresholdOverflowException: value=" + std::to_string(val)
               + " exceeded threshold=" + std::to_string(thresh)) {}
    const char* what()      const noexcept override { return msg_.c_str(); }
    double getValue()        const { return value_;     }
    double getThreshold()    const { return threshold_; }
};

// =============================================================================
// UNIT IV — Custom Stream Manipulators for tabular alert dispatch formatting
// =============================================================================

// Column-width manipulator
struct ColFmt {
    int w;
    explicit ColFmt(int width) : w(width) {}
};
std::ostream& operator<<(std::ostream& os, const ColFmt& c) {
    return os << std::left << std::setw(c.w);
}

// Table header / footer manipulators
std::ostream& alertHeader(std::ostream& os) {
    os << std::string(72, '=') << '\n'
       << ColFmt(5)  << "ID"
       << ColFmt(22) << "Station"
       << ColFmt(14) << "HazardIndex"
       << ColFmt(22) << "AlertLevel"
       << ColFmt(9)  << "Active" << '\n'
       << std::string(72, '-') << '\n';
    return os;
}
std::ostream& alertFooter(std::ostream& os) {
    return os << std::string(72, '=') << '\n';
}

// =============================================================================
// UNIT II — TelemetryBuffer (dynamic memory, deep copy, explicit destructor)
// =============================================================================

class TelemetryBuffer {
    std::string stationId_;
    int         capacity_;
    int         count_;
    double*     readings_;

public:
    // Parameterized constructor with dynamic allocation
    TelemetryBuffer(const std::string& id, int cap)
        : stationId_(id), capacity_(cap), count_(0) {
        readings_ = new double[capacity_]();
    }

    // Deep copy constructor
    TelemetryBuffer(const TelemetryBuffer& o)
        : stationId_(o.stationId_), capacity_(o.capacity_), count_(o.count_) {
        readings_ = new double[capacity_];
        std::copy(o.readings_, o.readings_ + capacity_, readings_);
    }

    // Copy assignment (deep)
    TelemetryBuffer& operator=(const TelemetryBuffer& o) {
        if (this != &o) {
            delete[] readings_;
            stationId_ = o.stationId_;
            capacity_  = o.capacity_;
            count_     = o.count_;
            readings_  = new double[capacity_];
            std::copy(o.readings_, o.readings_ + capacity_, readings_);
        }
        return *this;
    }

    // Explicit destructor — no memory leak
    ~TelemetryBuffer() { delete[] readings_; }

    bool push(double val) {
        if (count_ >= capacity_) return false;
        readings_[count_++] = val;
        return true;
    }

    double average() const {
        if (count_ == 0) return 0.0;
        double s = 0.0;
        for (int i = 0; i < count_; ++i) s += readings_[i];
        return s / count_;
    }

    double at(int i) const {
        if (i < 0 || i >= count_)
            throw SensorFailureException(-1, "Buffer index out of range");
        return readings_[i];
    }

    int  count()      const { return count_;     }
    int  capacity()   const { return capacity_;   }
    const std::string& id() const { return stationId_; }

    // Operator << (stream insertion)
    friend std::ostream& operator<<(std::ostream& os, const TelemetryBuffer& b) {
        os << "[TelemetryBuffer:" << b.stationId_ << " readings=" << b.count_
           << '/' << b.capacity_ << " avg=" << std::fixed << std::setprecision(2)
           << b.average() << ']';
        return os;
    }

    // Operator >> (stream extraction — reads space-separated doubles)
    friend std::istream& operator>>(std::istream& is, TelemetryBuffer& b) {
        double v;
        while (is >> v && b.count_ < b.capacity_) b.readings_[b.count_++] = v;
        return is;
    }
};

// =============================================================================
// UNIT II — EarlyWarningSiren (static members track network-wide state)
// =============================================================================

class EarlyWarningSiren {
    int         id_;
    std::string location_;
    bool        active_;
    DisasterCore::Alerts::Level currentAlert_;

    static int totalSirens_;
    static int activeSirens_;

public:
    EarlyWarningSiren(int id, const std::string& loc)
        : id_(id), location_(loc), active_(false),
          currentAlert_(DisasterCore::Alerts::Level::GREEN) {
        ++totalSirens_;
    }

    ~EarlyWarningSiren() {
        if (active_) --activeSirens_;
        --totalSirens_;
    }

    void activate(DisasterCore::Alerts::Level lv) {
        if (!active_) { active_ = true; ++activeSirens_; }
        currentAlert_ = lv;
        std::cout << "  [SIREN #" << id_ << " @ " << location_ << "] ACTIVATED → "
                  << DisasterCore::Alerts::levelToString(lv) << '\n';
    }

    void deactivate() {
        if (active_) { active_ = false; --activeSirens_; }
        currentAlert_ = DisasterCore::Alerts::Level::GREEN;
    }

    static int getTotalSirens()  { return totalSirens_;  }
    static int getActiveSirens() { return activeSirens_; }

    int  getId()      const { return id_;     }
    bool isActive()   const { return active_; }
    DisasterCore::Alerts::Level getAlert() const { return currentAlert_; }

    friend std::ostream& operator<<(std::ostream& os, const EarlyWarningSiren& s) {
        os << "Siren[" << s.id_ << '|' << s.location_ << '|'
           << (s.active_ ? "ACTIVE" : "IDLE") << '|'
           << DisasterCore::Alerts::levelToString(s.currentAlert_) << ']';
        return os;
    }
};
int EarlyWarningSiren::totalSirens_  = 0;
int EarlyWarningSiren::activeSirens_ = 0;

// =============================================================================
// UNIT II — SensorStation  (forward-declare TelemetryAuditor as friend)
// =============================================================================

class TelemetryAuditor; // forward declaration for friend

class SensorStation {
    int         id_;
    std::string name_;
    double      lat_, lon_;
    double      hazardIdx_;
    bool        operational_;
    TelemetryBuffer* buf_;           // dynamically allocated

    static int activeCount_;
    static int globalEmergencyLevel_;

    friend class TelemetryAuditor;  // ← UNIT I: friend class

public:
    SensorStation(int id, const std::string& name,
                  double lat, double lon, int bufCap = 50)
        : id_(id), name_(name), lat_(lat), lon_(lon),
          hazardIdx_(0.0), operational_(true) {
        buf_ = new TelemetryBuffer(name, bufCap);
        ++activeCount_;
    }

    // Deep copy constructor
    SensorStation(const SensorStation& o)
        : id_(o.id_), name_(o.name_), lat_(o.lat_), lon_(o.lon_),
          hazardIdx_(o.hazardIdx_), operational_(o.operational_) {
        buf_ = new TelemetryBuffer(*o.buf_);
        ++activeCount_;
    }

    // Copy assignment (deep)
    SensorStation& operator=(const SensorStation& o) {
        if (this != &o) {
            delete buf_;
            id_ = o.id_; name_ = o.name_; lat_ = o.lat_; lon_ = o.lon_;
            hazardIdx_ = o.hazardIdx_; operational_ = o.operational_;
            buf_ = new TelemetryBuffer(*o.buf_);
        }
        return *this;
    }

    ~SensorStation() { delete buf_; --activeCount_; }

    // ── Operator + : aggregate two stations into a regional zone vector ────
    std::vector<SensorStation*> operator+(SensorStation& rhs) {
        return { this, &rhs };
    }

    // ── Operator > : prioritise by higher hazard value ─────────────────────
    bool operator>(const SensorStation& rhs) const {
        return hazardIdx_ > rhs.hazardIdx_;
    }

    // ── Operator << : tabular row output ──────────────────────────────────
    friend std::ostream& operator<<(std::ostream& os, const SensorStation& ss) {
        os << std::fixed << std::setprecision(2)
           << ColFmt(5)  << ss.id_
           << ColFmt(22) << ss.name_
           << ColFmt(14) << ss.hazardIdx_
           << ColFmt(22) << DisasterCore::Alerts::levelToString(
                              DisasterCore::Alerts::hazardToLevel(ss.hazardIdx_))
           << ColFmt(9)  << (ss.operational_ ? "YES" : "NO");
        return os;
    }

    // ── Operator >> : ingest hazardIndex + coordinates from stream ────────
    friend std::istream& operator>>(std::istream& is, SensorStation& ss) {
        return is >> ss.hazardIdx_ >> ss.lat_ >> ss.lon_;
    }

    // Overload 1: rainfall + slope
    void ingestReading(double rainfall, double slope) {
        double hi = DisasterCore::Sensors::computeHazardIndex(rainfall, slope);
        hazardIdx_ = hi;
        buf_->push(hi);
        if (hi > 90.0) {
            ++globalEmergencyLevel_;
            throw ThresholdOverflowException(hi, 90.0);
        }
    }

    // Overload 2: soil + vibration + river level
    void ingestReading(double soil, double vibration, double riverLvl) {
        double hi = DisasterCore::Sensors::computeHazardIndex(soil, vibration, riverLvl);
        hazardIdx_ = hi;
        buf_->push(hi);
    }

    static int getActiveCount()         { return activeCount_;         }
    static int getGlobalEmergencyLevel(){ return globalEmergencyLevel_;}
    static void resetGlobalAlert()      { globalEmergencyLevel_ = 0;   }

    int         getId()           const { return id_;          }
    std::string getName()         const { return name_;         }
    double      getHazardIndex()  const { return hazardIdx_;    }
    double      getLatitude()     const { return lat_;          }
    double      getLongitude()    const { return lon_;          }
    bool        isOperational()   const { return operational_;  }
    TelemetryBuffer* getBuffer()  const { return buf_;          }
    void setOperational(bool v)         { operational_ = v;     }
};
int SensorStation::activeCount_         = 0;
int SensorStation::globalEmergencyLevel_ = 0;

// =============================================================================
// UNIT I — TelemetryAuditor (Friend Class)
//   Inspects raw private sensor metrics without exposing getters
// =============================================================================

class TelemetryAuditor {
public:
    static void auditStation(const SensorStation& ss) {
        // Direct access to private members via friend relationship
        std::cout << "  [Auditor] ID=" << ss.id_
                  << " Name=" << ss.name_
                  << " Lat="  << std::fixed << std::setprecision(4) << ss.lat_
                  << " Lon="  << ss.lon_
                  << " HazIdx="   << ss.hazardIdx_
                  << " BufAvg="   << ss.buf_->average()
                  << " Online="   << (ss.operational_ ? "YES" : "NO") << '\n';
    }

    static void auditNetwork(const std::vector<SensorStation*>& stations) {
        std::cout << "\n[TelemetryAuditor] Network-wide Audit — "
                  << SensorStation::activeCount_        << " active stations | "
                  << "GlobalAlert=" << SensorStation::globalEmergencyLevel_ << '\n';
        for (auto* s : stations) auditStation(*s);
    }
};

// =============================================================================
// UNIT III — PowerSubsystem (Containership / Aggregation inside BaseSensorNode)
// =============================================================================

class PowerSubsystem {
    double battery_pct_;
    double solar_W_;
    bool   mainsPowered_;

public:
    PowerSubsystem(double bat = 100.0, double solar = 15.0, bool mains = false)
        : battery_pct_(bat), solar_W_(solar), mainsPowered_(mains) {}

    void chargeCycle(double sunHours) {
        battery_pct_ = std::min(100.0, battery_pct_ + solar_W_ * sunHours * 0.5);
    }

    void consumePower(double watts, double hours) {
        double net = (watts - solar_W_) * hours;
        battery_pct_ = std::max(0.0, battery_pct_ - net * 0.1);
    }

    bool   hasSufficientPower() const { return battery_pct_ > 10.0; }
    double getBattery()         const { return battery_pct_;         }
    double getSolarInput()      const { return solar_W_;             }

    friend std::ostream& operator<<(std::ostream& os, const PowerSubsystem& ps) {
        os << "Power[Bat=" << std::fixed << std::setprecision(1) << ps.battery_pct_
           << "% Solar=" << ps.solar_W_ << "W Mains="
           << (ps.mainsPowered_ ? "YES" : "NO") << ']';
        return os;
    }
};

// =============================================================================
// UNIT III - Diamond Hierarchy (virtual base classes)
// Root: DisasterTelemetryDevice (virtual)
// Intermediate: SolarPoweredNode, WirelessLoRaTransceiver (virtually inherited)
// Leaf: AutonomousHybridProbe (single copy of root resolved by compiler)
// =============================================================================

// ── Root (shared single subobject via virtual inheritance) ────────────────
class DisasterTelemetryDevice {
protected:
    std::string serial_;
    double      lat_, lon_;
    bool        calibrated_;

public:
    DisasterTelemetryDevice(const std::string& serial, double lat, double lon)
        : serial_(serial), lat_(lat), lon_(lon), calibrated_(false) {}

    virtual ~DisasterTelemetryDevice() = default;

    virtual void calibrate() {
        calibrated_ = true;
        std::cout << "  [Device " << serial_ << "] Calibrated at ("
                  << lat_ << ", " << lon_ << ")\n";
    }

    const std::string& getSerial()  const { return serial_;    }
    bool               isCalibrated() const { return calibrated_; }
};

// ── Abstract Base (pure virtual interface + containership of PowerSubsystem) ─
class BaseSensorNode : public virtual DisasterTelemetryDevice {
protected:
    PowerSubsystem powerUnit_;   // ← CONTAINERSHIP (aggregation, independent object)
    int            nodeId_;
    std::string    nodeType_;

public:
    BaseSensorNode(int id, const std::string& type,
                   const std::string& serial, double lat, double lon,
                   const PowerSubsystem& ps = PowerSubsystem())
        : DisasterTelemetryDevice(serial, lat, lon),
          powerUnit_(ps), nodeId_(id), nodeType_(type) {}

    virtual ~BaseSensorNode() = default;

    // Pure virtual functions — resolved at runtime via vtable
    virtual double readTelemetry()      = 0;
    virtual bool   evaluateThreshold()  = 0;

    virtual void displayStatus() const {
        std::cout << "  Node[" << nodeId_ << '|' << nodeType_ << '|' << serial_
                  << "] " << powerUnit_ << '\n';
    }

    int               getNodeId()    const { return nodeId_;    }
    const std::string& getNodeType() const { return nodeType_;  }
    const PowerSubsystem& getPower() const { return powerUnit_; }
};

// ── Intermediate 1: SolarPoweredNode (virtual derivation) ─────────────────
class SolarPoweredNode : public virtual DisasterTelemetryDevice {
protected:
    double panelEfficiency_;
    double dailyEnergyWh_;

public:
    SolarPoweredNode(const std::string& serial, double lat, double lon,
                     double eff = 0.18)
        : DisasterTelemetryDevice(serial, lat, lon),
          panelEfficiency_(eff), dailyEnergyWh_(0.0) {}

    virtual ~SolarPoweredNode() = default;

    void harvestSolar(double sunHours) {
        dailyEnergyWh_ = panelEfficiency_ * sunHours * 10.0; // 10 W panel
        std::cout << "  [Solar " << serial_ << "] Harvested "
                  << std::fixed << std::setprecision(2)
                  << dailyEnergyWh_ << " Wh today\n";
    }

    double getPanelEfficiency() const { return panelEfficiency_; }
    double getDailyEnergy()     const { return dailyEnergyWh_;   }
};

// ── Intermediate 2: WirelessLoRaTransceiver (virtual derivation) ──────────
class WirelessLoRaTransceiver : public virtual DisasterTelemetryDevice {
protected:
    double freq_MHz_;
    double spreadFactor_;
    int    packetsSent_;

public:
    WirelessLoRaTransceiver(const std::string& serial, double lat, double lon,
                             double freq = 865.0, double sf = 10.0)
        : DisasterTelemetryDevice(serial, lat, lon),
          freq_MHz_(freq), spreadFactor_(sf), packetsSent_(0) {}

    virtual ~WirelessLoRaTransceiver() = default;

    void transmitPacket(const std::string& payload) {
        ++packetsSent_;
        std::cout << "  [LoRa @" << freq_MHz_ << "MHz SF=" << spreadFactor_
                  << "] Pkt#" << packetsSent_ << ": " << payload << '\n';
    }

    int    getPacketsSent() const { return packetsSent_; }
    double getFrequency()   const { return freq_MHz_;    }
};

// ── Leaf: AutonomousHybridProbe (diamond resolved; single DisasterTelemetryDevice)
class AutonomousHybridProbe : public BaseSensorNode,
                               public SolarPoweredNode,
                               public WirelessLoRaTransceiver {
    double soilMoisture_, seismicVibration_, riverLevel_;
    double rainfallRate_,  slopeAngle_;
    double lastHazardIdx_;

    static constexpr double THRESHOLD = 60.0;

public:
    AutonomousHybridProbe(int id, const std::string& serial,
                           double lat, double lon, double freq = 865.0)
        : DisasterTelemetryDevice(serial, lat, lon),           // virtual base first
          BaseSensorNode(id, "AutonomousHybridProbe", serial, lat, lon),
          SolarPoweredNode(serial, lat, lon),
          WirelessLoRaTransceiver(serial, lat, lon, freq),
          soilMoisture_(0), seismicVibration_(0), riverLevel_(0),
          rainfallRate_(0), slopeAngle_(0), lastHazardIdx_(0) {}

    // Inject simulated sensor readings
    void inject(double soil, double seismic, double river,
                double rain, double slope) {
        soilMoisture_    = soil;
        seismicVibration_= seismic;
        riverLevel_      = river;
        rainfallRate_    = rain;
        slopeAngle_      = slope;
    }

    // ── Override pure virtual: readTelemetry ─────────────────────────────
    double readTelemetry() override {
        if (!powerUnit_.hasSufficientPower())
            throw SensorFailureException(nodeId_, "Battery too low for telemetry read");
        lastHazardIdx_ = DisasterCore::Sensors::computeHazardIndex(
            rainfallRate_, slopeAngle_, soilMoisture_, seismicVibration_);
        return lastHazardIdx_;
    }

    // ── Override pure virtual: evaluateThreshold ──────────────────────────
    bool evaluateThreshold() override {
        if (lastHazardIdx_ > THRESHOLD) {
            transmitPacket("ALERT! HazardIdx=" + std::to_string(lastHazardIdx_));
            return true;
        }
        return false;
    }

    void displayStatus() const override {
        BaseSensorNode::displayStatus();
        std::cout << "  Soil=" << soilMoisture_ << "% Seismic=" << seismicVibration_
                  << "Hz River=" << riverLevel_ << "m Rain=" << rainfallRate_
                  << "mm/h Slope=" << slopeAngle_ << "° HazIdx=" << lastHazardIdx_ << '\n';
    }

    double getLastHazardIdx() const { return lastHazardIdx_; }
};

// =============================================================================
// UNIT IV — Persistent Binary Incident Logger (binary fstream + seekp/seekg)
// =============================================================================

struct IncidentRecord {
    int      stationId;
    double   hazardIndex;
    double   latitude;
    double   longitude;
    char     alertLevel[32];
    char     timestamp[32];
    bool     rolledBack;
};

class IncidentLogger {
    std::string   filename_;
    std::fstream  file_;
    std::streampos commitPos_;

public:
    explicit IncidentLogger(const std::string& fname) : filename_(fname) {
        file_.open(fname, std::ios::in | std::ios::out |
                           std::ios::binary | std::ios::trunc);
        if (!file_.is_open())
            throw std::runtime_error("IncidentLogger: cannot open " + fname);
        commitPos_ = 0;
    }

    ~IncidentLogger() { if (file_.is_open()) file_.close(); }

    // Mark current end-of-file as commit point
    void beginTransaction() {
        file_.seekp(0, std::ios::end);
        commitPos_ = file_.tellp();
    }

    // Append a record (binary write)
    void logIncident(const IncidentRecord& rec) {
        file_.seekp(0, std::ios::end);
        file_.write(reinterpret_cast<const char*>(&rec), sizeof(IncidentRecord));
        file_.flush();
        std::cout << "  [Logger] Written: Station#" << rec.stationId
                  << " HazardIdx=" << rec.hazardIndex
                  << " Alert=" << rec.alertLevel << '\n';
    }

    // Rollback: seek back to commit point and mark any written records as rolled-back
    void rollback() {
        // Find how many records were written since last commit
        file_.seekp(0, std::ios::end);
        std::streampos endPos = file_.tellp();
        long long written = static_cast<long long>(endPos - commitPos_);
        int numToRollback = static_cast<int>(written / sizeof(IncidentRecord));

        // seekg to commitPos, re-read and mark each post-commit record as rolledBack
        for (int i = 0; i < numToRollback; ++i) {
            std::streampos recPos = commitPos_
                + static_cast<std::streamoff>(i * sizeof(IncidentRecord));
            IncidentRecord r;
            file_.seekg(recPos);
            file_.read(reinterpret_cast<char*>(&r), sizeof(IncidentRecord));
            r.rolledBack = true;
            file_.seekp(recPos);
            file_.write(reinterpret_cast<const char*>(&r), sizeof(IncidentRecord));
        }
        file_.flush();
        std::cout << "  [Logger] ROLLBACK: marked " << numToRollback
                  << " record(s) as rolled-back (seekp/seekg)\n";
    }

    void commit() {
        file_.flush();
        file_.seekp(0, std::ios::end);
        commitPos_ = file_.tellp();
        std::cout << "  [Logger] COMMIT  at position " << (long long)commitPos_ << '\n';
    }

    // Read all non-rolled-back records using seekg
    std::vector<IncidentRecord> readAll() {
        std::vector<IncidentRecord> recs;
        file_.clear();
        file_.seekg(0, std::ios::beg);
        IncidentRecord r;
        while (file_.read(reinterpret_cast<char*>(&r), sizeof(IncidentRecord)))
            if (!r.rolledBack) recs.push_back(r);
        file_.clear();
        return recs;
    }
};

// =============================================================================
// UNIT IV — Custom Functors for priority-based evacuation routing
// =============================================================================

struct EvacuationRoute {
    std::string zone;
    double      hazardLevel;
    int         population;
    std::string road;
};

// Functor: sort descending by hazard level
struct HazardPriorityComparator {
    bool operator()(const EvacuationRoute& a, const EvacuationRoute& b) const {
        return a.hazardLevel > b.hazardLevel;
    }
};

// Functor: dispatch alert if above minimum level
struct AlertDispatcher {
    DisasterCore::Alerts::Level minLevel;
    int& dispatched;

    AlertDispatcher(DisasterCore::Alerts::Level min, int& cnt)
        : minLevel(min), dispatched(cnt) {}

    void operator()(const EvacuationRoute& r) const {
        auto lv = DisasterCore::Alerts::hazardToLevel(r.hazardLevel);
        if (static_cast<int>(lv) >= static_cast<int>(minLevel)) {
            std::cout << "  [DISPATCH] " << std::left
                      << ColFmt(20) << r.zone
                      << " Hazard=" << ColFmt(7) << r.hazardLevel
                      << " Pop=" << ColFmt(7) << r.population
                      << " Via: " << r.road << '\n';
            ++dispatched;
        }
    }
};

// =============================================================================
// UNIT IV — RegionalSweepEngine (std::vector<BaseSensorNode*>, vtable dispatch)
// =============================================================================

class RegionalSweepEngine {
    std::vector<BaseSensorNode*>   nodes_;
    std::map<int, double>          hazardMap_;    // nodeId → hazardIndex
    std::map<int, std::string>     alertMap_;     // nodeId → alert string

public:
    void registerNode(BaseSensorNode* n) { nodes_.push_back(n); }

    // Cyclical sweep — runtime polymorphism via vtable/_vptr binding
    void cyclicalSweep() {
        for (auto* n : nodes_) {
            try {
                double hi      = n->readTelemetry();
                bool triggered = n->evaluateThreshold();
                auto  lv       = DisasterCore::Alerts::hazardToLevel(hi);
                hazardMap_[n->getNodeId()] = hi;
                alertMap_[n->getNodeId()]  = DisasterCore::Alerts::levelToString(lv);
                std::cout << "  Node[" << n->getNodeId() << "] HazardIdx="
                          << std::fixed << std::setprecision(2) << hi
                          << " → " << DisasterCore::Alerts::levelToString(lv)
                          << (triggered ? "  ★ THRESHOLD TRIGGERED" : "") << '\n';
            }
            catch (const SensorFailureException& e) {
                std::cerr << "  SENSOR ERR: " << e.what() << '\n';
            }
            catch (const ThresholdOverflowException& e) {
                std::cerr << "  OVERFLOW  : " << e.what() << '\n';
            }
        }
    }

    // STL std::sort on hazard ranking
    std::vector<std::pair<int,double>> rankedHazards() const {
        std::vector<std::pair<int,double>> v(hazardMap_.begin(), hazardMap_.end());
        std::sort(v.begin(), v.end(),
            [](const auto& a, const auto& b){ return a.second > b.second; });
        return v;
    }

    // STL std::for_each to print hazard registry
    void printHazardMap() const {
        std::for_each(hazardMap_.begin(), hazardMap_.end(),
            [&](const auto& e) {
                std::cout << "  Node[" << e.first << "] → "
                          << std::fixed << std::setprecision(2) << e.second
                          << " (" << alertMap_.at(e.first) << ")\n";
            });
    }
};

// =============================================================================
// MAIN — Unified ALF-EWG system run (all features merged into one flow)
// =============================================================================

int main() {
    std::cout << std::string(72, '=') << '\n'
              << "  ALF-EWG: Autonomous Landslide & Flash-Flood Early Warning Grid\n"
              << std::string(72, '=') << "\n\n";

    // ── Step 1: Boot sensor stations & compute initial hazard indices ─────
    std::cout << "--- Booting Sensor Stations ---\n";

    SensorStation s1(1, "Kedarnath-North", 30.734, 79.066, 20);
    SensorStation s2(2, "Badrinath-East",  30.744, 79.493, 20);
    SensorStation s3(3, "Chamoli-Valley",  30.400, 79.350, 20);

    s1.ingestReading(85.0, 40.0);       // rainfall=85mm/h, slope=40 deg
    s2.ingestReading(72.0, 6.0, 3.8);  // soil=72%, vibr=6Hz, river=3.8m
    try {
        s3.ingestReading(110.0, 65.0); // rainfall=110, slope=65 -> hi=92.0 > 90 -> triggers exception!
    } catch (const ThresholdOverflowException& e) {
        std::cout << "  [Alert] Ingestion warning: " << e.what() << '\n';
    }

    // ── Step 2: Activate sirens based on alert levels ─────────────────────
    std::cout << "\n--- Activating Early Warning Sirens ---\n";
    EarlyWarningSiren siren1(1, "Kedarnath-Village");
    EarlyWarningSiren siren2(2, "Badrinath-Gate");
    EarlyWarningSiren siren3(3, "Chamoli-Township");
    siren1.activate(DisasterCore::Alerts::hazardToLevel(s1.getHazardIndex()));
    siren2.activate(DisasterCore::Alerts::hazardToLevel(s2.getHazardIndex()));
    siren3.activate(DisasterCore::Alerts::hazardToLevel(s3.getHazardIndex()));
    std::cout << "  Network: " << EarlyWarningSiren::getTotalSirens()
              << " sirens total, " << EarlyWarningSiren::getActiveSirens()
              << " active | Stations online: " << SensorStation::getActiveCount() << '\n';

    // ── Step 3: Regional zone table ───────────────────────────────────────
    std::cout << "\n--- Regional Station Status ---\n";
    auto zone = s1 + s2;
    zone.push_back(&s3);
    alertHeader(std::cout);
    for (auto* st : zone) std::cout << *st << '\n';
    alertFooter(std::cout);

    // ── Step 4: Hazard index calculations (overloaded inline functions) ───
    std::cout << "--- Hazard Index Calculations ---\n";
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  computeHazardIndex(rainfall=85, slope=42)          = "
              << DisasterCore::Sensors::computeHazardIndex(85.0, 42.0) << '\n';
    std::cout << "  computeHazardIndex(soil=78, vibr=5.5, river=3.2)  = "
              << DisasterCore::Sensors::computeHazardIndex(78.0, 5.5, 3.2) << '\n';
    std::cout << "  computeHazardIndex(rain=70, slope=35, sat=65, v=4) = "
              << DisasterCore::Sensors::computeHazardIndex(70.0, 35.0, 65.0, 4.0) << '\n';

    // Geocoord transform (call by reference) & accumulator (return by reference)
    double lat = 30.3165, lon = 78.0322;
    DisasterCore::Sensors::transformGeoCoordinates(lat, lon, 0.001, 0.002);
    std::cout << "  Geocoord transform → (" << lat << ", " << lon << ")\n";
    double acc = 0.0;
    DisasterCore::Sensors::accumulateTelemetry(acc, 55.3);
    DisasterCore::Sensors::accumulateTelemetry(acc, 62.1);
    std::cout << "  Telemetry accumulator → " << acc << "\n\n";

    // ── Step 5: Telemetry buffer (dynamic memory, deep copy, >>/<<) ───────
    std::cout << "--- Telemetry Buffer ---\n";
    TelemetryBuffer buf("KDN-Stream", 10);
    std::istringstream iss("11.1 22.2 33.3 44.4");
    iss >> buf;
    std::cout << "  " << buf << '\n';
    TelemetryBuffer bufCopy = buf;  // deep copy
    std::cout << "  Deep copy → " << bufCopy << "\n\n";

    // ── Step 6: TelemetryAuditor (friend class — raw private access) ─────
    std::cout << "--- TelemetryAuditor (Friend Class) ---\n";
    std::vector<SensorStation*> stPtrs = {&s1, &s2, &s3};
    TelemetryAuditor::auditNetwork(stPtrs);
    std::cout << '\n';

    // ── Step 7: Calibrate & deploy AutonomousHybridProbes (diamond) ──────
    std::cout << "--- Calibrating Autonomous Hybrid Probes (Diamond Hierarchy) ---\n";
    AutonomousHybridProbe probe1(101, "AHP-KDN-01", 30.734, 79.066, 865.0);
    AutonomousHybridProbe probe2(102, "AHP-BDR-02", 30.744, 79.493, 866.0);
    AutonomousHybridProbe probe3(103, "AHP-CHM-03", 30.400, 79.350, 867.0);
    probe1.calibrate();
    probe2.calibrate();
    probe3.calibrate();

    probe1.inject(78.0, 4.5, 2.8, 80.0, 38.0);
    probe2.inject(55.0, 3.0, 1.5, 60.0, 25.0);
    probe3.inject(90.0, 7.2, 4.1, 92.0, 48.0);

    probe1.harvestSolar(6.0);
    probe2.harvestSolar(4.5);

    // ── Step 8: Cyclical sensor sweep (vtable polymorphic dispatch) ───────
    std::cout << "\n--- Cyclical Sensor Sweep (Polymorphic vtable Dispatch) ---\n";
    RegionalSweepEngine engine;
    engine.registerNode(&probe1);
    engine.registerNode(&probe2);
    engine.registerNode(&probe3);
    engine.cyclicalSweep();

    // ── Step 9: Hazard map + ranking ──────────────────────────────────────
    std::cout << "\n--- Live Hazard Registry ---\n";
    engine.printHazardMap();

    std::cout << "\n--- Hazard Priority Ranking ---\n";
    for (const auto& entry : engine.rankedHazards())
        std::cout << "  Node[" << entry.first << "] → "
                  << std::fixed << std::setprecision(2) << entry.second << '\n';

    // ── Step 10: Probe power & sensor status ─────────────────────────────
    std::cout << "\n--- Probe Status (Containership: PowerSubsystem) ---\n";
    probe1.displayStatus();
    probe2.displayStatus();
    probe3.displayStatus();

    // ── Step 11: Binary incident logging + transactional rollback ────────
    std::cout << "\n--- Incident Logging (Binary fstream) ---\n";
    try {
        IncidentLogger logger("incidents.bin");

        auto makeRecord = [](int id, double hi, double lt, double ln,
                              const std::string& al, const std::string& ts) {
            IncidentRecord r{};
            r.stationId   = id;  r.hazardIndex = hi;
            r.latitude    = lt;  r.longitude   = ln;
            r.rolledBack  = false;
            std::strncpy(r.alertLevel, al.c_str(), 31);
            std::strncpy(r.timestamp,  ts.c_str(), 31);
            return r;
        };

        logger.beginTransaction();
        logger.logIncident(makeRecord(1, s1.getHazardIndex(),
            s1.getLatitude(), s1.getLongitude(),
            DisasterCore::Alerts::levelToString(
                DisasterCore::Alerts::hazardToLevel(s1.getHazardIndex())),
            "2026-11-11T15:00:00"));
        logger.logIncident(makeRecord(2, s2.getHazardIndex(),
            s2.getLatitude(), s2.getLongitude(),
            DisasterCore::Alerts::levelToString(
                DisasterCore::Alerts::hazardToLevel(s2.getHazardIndex())),
            "2026-11-11T15:05:00"));
        logger.commit();

        logger.beginTransaction();
        logger.logIncident(makeRecord(99, 999.9, 0, 0, "CORRUPT", "CORRUPT"));
        std::cout << "  Corrupt write detected → rolling back...\n";
        logger.rollback();

        auto recs = logger.readAll();
        std::cout << "  Verified records on disk: " << recs.size() << '\n';
        for (auto& r : recs)
            std::cout << "    Station#" << r.stationId
                      << "  HazardIdx=" << std::fixed << std::setprecision(1) << r.hazardIndex
                      << "  Alert=" << r.alertLevel
                      << "  Time=" << r.timestamp << '\n';
    }
    catch (const std::exception& e) {
        std::cerr << "  Logger error: " << e.what() << '\n';
    }

    // ── Step 12: Evacuation routing (custom functors + STL) ───────────────
    std::cout << "\n--- Evacuation Route Dispatch ---\n";
    std::vector<EvacuationRoute> routes = {
        {"Kedarnath-Zone-A",  85.0, 1200, "NH-58 via Gaurikund"},
        {"Badrinath-Zone-B",  62.0,  800, "NH-7 via Joshimath"},
        {"Chamoli-Zone-C",    91.0, 2500, "SH-13 via Gopeshwar"},
        {"Rudraprayag-Zone-D",45.0,  600, "NH-58 via Tilwara"},
        {"Pauri-Zone-E",      30.0,  400, "NH-119 via Kotdwar"},
    };
    std::sort(routes.begin(), routes.end(), HazardPriorityComparator{});
    int dispatched = 0;
    std::for_each(routes.begin(), routes.end(),
        AlertDispatcher{DisasterCore::Alerts::Level::ORANGE, dispatched});
    std::cout << "  Zones dispatched: " << dispatched << " / " << routes.size() << '\n';

    // ── Step 13: Zone alert summary ───────────────────────────────────────
    std::cout << "\n--- Zone Alert Summary ---\n";
    std::map<std::string, int> summary;
    for (const auto& r : routes)
        summary[DisasterCore::Alerts::levelToString(
            DisasterCore::Alerts::hazardToLevel(r.hazardLevel))]++;
    std::for_each(summary.begin(), summary.end(), [](const auto& p) {
        std::cout << "  " << std::left << std::setw(26) << p.first
                  << ": " << p.second << " zone(s)\n";
    });

    // ── Step 14: Final network-wide station report ─────────────────────────
    std::cout << "\n--- Final Station Report ---\n";
    alertHeader(std::cout);
    for (auto* st : stPtrs) std::cout << *st << '\n';
    alertFooter(std::cout);

    std::cout << std::string(72, '=') << '\n'
              << "  ALF-EWG System shutdown complete. All heap memory released.\n"
              << std::string(72, '=') << '\n';
    return 0;
}
