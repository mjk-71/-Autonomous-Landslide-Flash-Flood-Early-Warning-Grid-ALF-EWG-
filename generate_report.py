import os
import sys
from reportlab.lib.pagesizes import letter
from reportlab.lib import colors
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
)
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.pdfgen import canvas

class NumberedCanvas(canvas.Canvas):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_page_number(num_pages)
            super().showPage()
        super().save()

    def draw_page_number(self, page_count):
        if self._pageNumber == 1:
            return  # Suppress running header/footer on cover page
        self.saveState()
        self.setFont("Helvetica", 8)
        self.setFillColor(colors.HexColor("#4A5568"))
        
        # Running header
        self.drawString(54, 11 * 72 - 36, "MAIT CSE | CIC-211 OOP Using C++ | Project Report: ALF-EWG")
        self.setStrokeColor(colors.HexColor("#CBD5E0"))
        self.setLineWidth(0.5)
        self.line(54, 11 * 72 - 42, 8.5 * 72 - 54, 11 * 72 - 42)
        
        # Running footer
        page_text = f"Page {self._pageNumber} of {page_count}"
        self.drawRightString(8.5 * 72 - 54, 36, page_text)
        self.drawString(54, 36, "Confidential - Academic Evaluation Copy (CIC-211)")
        self.line(54, 46, 8.5 * 72 - 54, 46)
        self.restoreState()

def build_pdf(filename):
    doc = SimpleDocTemplate(
        filename,
        pagesize=letter,
        leftMargin=54,
        rightMargin=54,
        topMargin=54,
        bottomMargin=54
    )

    styles = getSampleStyleSheet()

    # Custom color palette
    c_primary = colors.HexColor("#1A365D")   # Deep Navy
    c_secondary = colors.HexColor("#2B6CB0") # Cobalt Blue
    c_accent = colors.HexColor("#C53030")    # Crimson Accent
    c_dark = colors.HexColor("#2D3748")      # Charcoal Text
    c_light_bg = colors.HexColor("#EDF2F7")  # Cool Grey Background
    c_border = colors.HexColor("#E2E8F0")    # Table Border

    # Typography styles
    title_style = ParagraphStyle(
        'CoverTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=20,
        leading=24,
        textColor=c_primary,
        alignment=1, # Center
        spaceAfter=8
    )

    subtitle_style = ParagraphStyle(
        'CoverSubtitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=12,
        leading=16,
        textColor=c_secondary,
        alignment=1,
        spaceAfter=15
    )

    institution_style = ParagraphStyle(
        'Institution',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=10,
        leading=14,
        textColor=c_dark,
        alignment=1,
        spaceAfter=25
    )

    h1_style = ParagraphStyle(
        'Heading1_Custom',
        parent=styles['Heading1'],
        fontName='Helvetica-Bold',
        fontSize=13,
        leading=17,
        textColor=c_primary,
        spaceBefore=14,
        spaceAfter=6,
        keepWithNext=True
    )

    h2_style = ParagraphStyle(
        'Heading2_Custom',
        parent=styles['Heading2'],
        fontName='Helvetica-Bold',
        fontSize=10.5,
        leading=14,
        textColor=c_secondary,
        spaceBefore=10,
        spaceAfter=4,
        keepWithNext=True
    )

    body_style = ParagraphStyle(
        'Body_Custom',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.5,
        leading=12,
        textColor=c_dark,
        spaceAfter=6
    )

    bullet_style = ParagraphStyle(
        'Bullet_Custom',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.5,
        leading=12,
        textColor=c_dark,
        leftIndent=15,
        spaceAfter=3
    )

    code_style = ParagraphStyle(
        'Code_Custom',
        parent=styles['Normal'],
        fontName='Courier',
        fontSize=7.5,
        leading=9.5,
        textColor=colors.HexColor("#1A202C")
    )

    table_header_style = ParagraphStyle(
        'TableHeader',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=8,
        leading=10,
        textColor=colors.white
    )

    table_cell_style = ParagraphStyle(
        'TableCell',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=7.5,
        leading=10,
        textColor=c_dark
    )

    callout_style = ParagraphStyle(
        'CalloutText',
        parent=styles['Normal'],
        fontName='Helvetica-Oblique',
        fontSize=8,
        leading=11,
        textColor=colors.HexColor("#2C5282")
    )

    story = []

    # =========================================================================
    # COVER / HEADER BLOCK
    # =========================================================================
    story.append(Paragraph("MAHARAJA AGRASEN INSTITUTE OF TECHNOLOGY", ParagraphStyle(
        'MaitHeader', fontName='Helvetica-Bold', fontSize=13, leading=16, alignment=1, textColor=c_primary
    )))
    story.append(Paragraph("Department of Computer Science and Engineering", ParagraphStyle(
        'DeptHeader', fontName='Helvetica', fontSize=10, leading=13, alignment=1, textColor=c_secondary, spaceAfter=4
    )))
    story.append(Paragraph("Subject: Object-Oriented Programming Using C++ (Paper Code: CIC-211)", ParagraphStyle(
        'SubHeader', fontName='Helvetica-Bold', fontSize=9, leading=12, alignment=1, textColor=c_dark, spaceAfter=15
    )))
    story.append(HRFlowable(width="100%", thickness=1.5, color=c_primary, spaceAfter=15))

    story.append(Paragraph("PROJECT TECHNICAL DESIGN & IMPLEMENTATION REPORT", ParagraphStyle(
        'DocType', fontName='Helvetica-Bold', fontSize=10, leading=13, alignment=1, textColor=c_accent, spaceAfter=6
    )))
    story.append(Paragraph("Autonomous Landslide & Flash-Flood Early Warning Grid<br/>(ALF-EWG)", title_style))
    story.append(Paragraph("A High-Performance C++ Object-Oriented IoT Telemetry & Disaster Management System", subtitle_style))

    # Meta Info Card
    meta_data = [
        [
            Paragraph("<b>Project Option:</b> Option 1 (ALF-EWG)", table_cell_style),
            Paragraph("<b>Standard:</b> Clean ISO C++14/17 Standard", table_cell_style)
        ],
        [
            Paragraph("<b>Course Code:</b> CIC-211 (OOP Using C++)", table_cell_style),
            Paragraph("<b>Target Domain:</b> Himalayan Disaster Risk Management", table_cell_style)
        ],
        [
            Paragraph("<b>Source File:</b> <code>ALF-EWG.cpp</code>", table_cell_style),
            Paragraph("<b>Submission Date:</b> November 2026", table_cell_style)
        ],
        [
            Paragraph("<b>Architecture:</b> 4-Unit Syllabus Compliant", table_cell_style),
            Paragraph("<b>Execution Status:</b> Verified (Exit Code 0)", table_cell_style)
        ]
    ]
    meta_table = Table(meta_data, colWidths=[240, 260])
    meta_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), c_light_bg),
        ('BOX', (0,0), (-1,-1), 1, c_secondary),
        ('INNERGRID', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E0")),
        ('TOPPADDING', (0,0), (-1,-1), 5),
        ('BOTTOMPADDING', (0,0), (-1,-1), 5),
        ('LEFTPADDING', (0,0), (-1,-1), 8),
        ('RIGHTPADDING', (0,0), (-1,-1), 8),
    ]))
    story.append(meta_table)
    story.append(Spacer(1, 15))

    # =========================================================================
    # SECTION 1: SYSTEM OVERVIEW & DOMAIN CONTEXT
    # =========================================================================
    story.append(Paragraph("1. Domain & System Context", h1_style))
    story.append(Paragraph(
        "In vulnerable mountainous regions such as the Himalayan belt, extreme precipitation and seismic instability trigger sudden catastrophic landslides and flash floods, crippling transportation corridors, electric grids, and residential settlements. The Autonomous Landslide & Flash-Flood Early Warning Grid (ALF-EWG) is designed as a mission-critical edge-and-backend telemetry ingestion system deployed across remote solar-powered IoT monitoring stations.",
        body_style
    ))
    story.append(Paragraph(
        "ALF-EWG autonomously collects multi-modal sensor streams (pore pressure, soil moisture, geophone vibrations, tiltmeters, and river-level sonar gauges), continuously calculates hazard indexes in real time, issues multi-tier regional siren alerts, executes priority-based emergency evacuation routing, and atomically logs all incidents to persistent binary disk storage with transactional rollback support.",
        body_style
    ))

    # Key Architectural Principles Callout
    callout_data = [[
        Paragraph(
            "<b>Key Architectural Principles:</b><br/>"
            "• <b>Strict Encapsulation:</b> Separation of raw sensor drivers from processing alerts via dedicated namespaces.<br/>"
            "• <b>Zero Memory Leaks:</b> RAII resource management across dynamically allocated sensor circular buffers.<br/>"
            "• <b>Ambiguity-Free Multiple Inheritance:</b> Virtual base classes resolving the classical C++ Diamond Problem.<br/>"
            "• <b>Exception-Safe Persistence:</b> Atomic file logging supporting commit/rollback points via stream position manipulation.",
            callout_style
        )
    ]]
    callout_table = Table(callout_data, colWidths=[500])
    callout_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#EBF8FF")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#3182CE")),
        ('TOPPADDING', (0,0), (-1,-1), 6),
        ('BOTTOMPADDING', (0,0), (-1,-1), 6),
        ('LEFTPADDING', (0,0), (-1,-1), 10),
        ('RIGHTPADDING', (0,0), (-1,-1), 10),
    ]))
    story.append(callout_table)
    story.append(Spacer(1, 10))

    # =========================================================================
    # SECTION 2: ARCHITECTURAL DESIGN & DIAMOND RESOLUTION
    # =========================================================================
    story.append(Paragraph("2. System Architecture & Object-Oriented Design", h1_style))
    story.append(Paragraph(
        "The software architecture enforces clean separation between classification hierarchies and component containment, directly addressing the core competencies of the CIC-211 curriculum.",
        body_style
    ))

    # Diamond problem explanation & ASCII / diagram
    story.append(Paragraph("2.1 Virtual Base Classes & Diamond Problem Resolution", h2_style))
    story.append(Paragraph(
        "When an autonomous IoT probe inherits both solar harvesting capabilities (<code>SolarPoweredNode</code>) and wireless LoRa communication telemetry (<code>WirelessLoRaTransceiver</code>), both branches derive from the common root hardware abstraction <code>DisasterTelemetryDevice</code>. Without virtual inheritance, the leaf probe <code>AutonomousHybridProbe</code> would inherit duplicate copies of the root device state (serial number, calibration status, base geocoordinates), resulting in compile-time ambiguity.",
        body_style
    ))

    diamond_code = """
                    +-----------------------------+
                    |   DisasterTelemetryDevice   |  <-- Root (Virtual Base Class)
                    | (serial, lat, lon, calibrate)|
                    +-----------------------------+
                                /                 \\
                               /                   \\  (virtual inheritance)
                              /                     \\
         +------------------------+      +---------------------------+
         |    SolarPoweredNode    |      |  WirelessLoRaTransceiver  |
         | (harvestSolar, energy) |      | (freq_MHz, transmitPacket)|
         +------------------------+      +---------------------------+
                              \\                     /
                               \\                   /  (multiple inheritance)
                                \\                 /
                    +------------------------------------+
                    |       AutonomousHybridProbe        |  <-- Leaf Probe (Unified Subobject)
                    | (readTelemetry, evaluateThreshold) |
                    +------------------------------------+
    """
    diamond_table = Table([[Paragraph(f"<pre>{diamond_code.strip()}</pre>", code_style)]], colWidths=[500])
    diamond_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#1A202C")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#4A5568")),
        ('TOPPADDING', (0,0), (-1,-1), 6),
        ('BOTTOMPADDING', (0,0), (-1,-1), 6),
        ('LEFTPADDING', (0,0), (-1,-1), 12),
        ('RIGHTPADDING', (0,0), (-1,-1), 12),
    ]))
    # Adjust font color in pre
    p_code = Paragraph(f"<font color='#E2E8F0'><pre>{diamond_code.strip()}</pre></font>", code_style)
    diamond_table = Table([[p_code]], colWidths=[500])
    diamond_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#1A202C")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#4A5568")),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
        ('LEFTPADDING', (0,0), (-1,-1), 10),
        ('RIGHTPADDING', (0,0), (-1,-1), 10),
    ]))
    story.append(diamond_table)
    story.append(Spacer(1, 6))

    story.append(Paragraph(
        "<b>Virtual Base Resolution Mechanism:</b> By declaring <code>class SolarPoweredNode : public virtual DisasterTelemetryDevice</code> and <code>class WirelessLoRaTransceiver : public virtual DisasterTelemetryDevice</code>, the C++ runtime delegates construction of the shared subobject directly to the most-derived leaf constructor (<code>AutonomousHybridProbe</code>). This guarantees exactly one copy of the root state in memory.",
        body_style
    ))

    story.append(Paragraph("2.2 Containership (Aggregation) vs. Inheritance", h2_style))
    story.append(Paragraph(
        "To satisfy the design principle of 'favoring composition over inheritance', <code>PowerSubsystem</code> is modeled as an independent, encapsulated class and aggregated directly inside <code>BaseSensorNode</code> (containership). A sensor node <i>has a</i> power unit rather than <i>is a</i> power unit. This allows battery discharge cycles, solar recharging, and low-power threshold evaluations to be decoupled from sensor telemetry logic.",
        body_style
    ))

    # =========================================================================
    # SECTION 3: COMPREHENSIVE SYLLABUS UNIT MAPPING
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("3. Technical Requirements Mapping Across Syllabus", h1_style))
    story.append(Paragraph(
        "Every technical specification detailed in the CIC-211 Project Assignment has been rigorously fulfilled in the codebase. The following sections describe the mechanism, theoretical significance, and concrete implementation.",
        body_style
    ))

    # --- Unit I ---
    story.append(Paragraph("3.1 Unit I: Core Foundations & Call Semantics", h2_style))
    
    u1_items = [
        ("Distinct Namespaces", "DisasterCore::Sensors and DisasterCore::Alerts partition mathematical processing from notification state dispatch, preventing identifier collisions."),
        ("Inline Mathematical Risk Functions", "computeHazardIndex() algorithms are declared inline to eliminate stack-frame call overhead during microsecond-level telemetry processing loops."),
        ("Function Overloading", "computeHazardIndex() is overloaded with 3 specialized signatures: (1) rainfall rate + slope angle; (2) soil saturation + vibration + river sonar; (3) all 4 parameters combined."),
        ("Call by Reference", "transformGeoCoordinates(double& lat, double& lon, double dLat, double dLon) accepts references to avoid copying coordinate structs during high-frequency GPS transforms."),
        ("Return by Reference", "accumulateTelemetry(double& accumulator, double reading) returns a lvalue double& to allow chained state mutations and immediate cumulative state tracking without temporary copies."),
        ("Friend Class Audit", "TelemetryAuditor is declared as a friend class of SensorStation, granting access to private telemetry buffers and calibration offsets without violating encapsulation via public getters.")
    ]
    u1_data = [[Paragraph(f"<b>{k}</b>", table_cell_style), Paragraph(v, table_cell_style)] for k, v in u1_items]
    t1 = Table([[Paragraph("Requirement", table_header_style), Paragraph("Implementation & Technical Role", table_header_style)]] + u1_data, colWidths=[150, 350])
    t1.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), c_primary),
        ('BOX', (0,0), (-1,-1), 1, c_border),
        ('INNERGRID', (0,0), (-1,-1), 0.5, c_border),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
        ('LEFTPADDING', (0,0), (-1,-1), 6),
        ('RIGHTPADDING', (0,0), (-1,-1), 6),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, c_light_bg]),
    ]))
    story.append(t1)
    story.append(Spacer(1, 10))

    # --- Unit II ---
    story.append(Paragraph("3.2 Unit II: Encapsulation, Dynamic Memory & Operator Overloading", h2_style))
    u2_items = [
        ("Entities & Const Correctness", "SensorStation, EarlyWarningSiren, and TelemetryBuffer maintain strict information hiding. Member accessors (average(), getId()) are declared const."),
        ("Static Data & Member Functions", "SensorStation::activeCount_ and SensorStation::globalEmergencyLevel_ along with EarlyWarningSiren::totalSirens_ track distributed network state across all instances."),
        ("Dynamic Constructors & Deep Copy", "TelemetryBuffer dynamically allocates double* readings_ in its parameterized constructor. Explicit copy constructor and copy assignment operator perform deep copies, preventing double-free corruption."),
        ("Operator Overloading (+)", "operator+(SensorStation& rhs) aggregates two or more sensor stations into a regional zone vector (std::vector<SensorStation*>)."),
        ("Operator Overloading (>)", "operator>(const SensorStation& rhs) provides natural relational comparison, prioritizing stations with higher hazard index values for triage ordering."),
        ("Operator Overloading (<<, >>)", "operator<< outputs tabular row formatting for console and file streams; operator>> extracts telemetry streams into stations and buffers.")
    ]
    u2_data = [[Paragraph(f"<b>{k}</b>", table_cell_style), Paragraph(v, table_cell_style)] for k, v in u2_items]
    t2 = Table([[Paragraph("Requirement", table_header_style), Paragraph("Implementation & Technical Role", table_header_style)]] + u2_data, colWidths=[150, 350])
    t2.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), c_secondary),
        ('BOX', (0,0), (-1,-1), 1, c_border),
        ('INNERGRID', (0,0), (-1,-1), 0.5, c_border),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
        ('LEFTPADDING', (0,0), (-1,-1), 6),
        ('RIGHTPADDING', (0,0), (-1,-1), 6),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, c_light_bg]),
    ]))
    story.append(t2)
    story.append(Spacer(1, 10))

    # --- Unit III ---
    story.append(Paragraph("3.3 Unit III: Inheritance, Containership & Polymorphism", h2_style))
    u3_items = [
        ("Abstract Base Class", "BaseSensorNode declares pure virtual functions readTelemetry() = 0 and evaluateThreshold() = 0, enforcing a strict polymorphic interface."),
        ("Virtual Base Classes", "DisasterTelemetryDevice serves as virtual base class for SolarPoweredNode and WirelessLoRaTransceiver, resolving the diamond inheritance in AutonomousHybridProbe."),
        ("Containership (Aggregation)", "BaseSensorNode aggregates an independent PowerSubsystem instance (battery level, solar charging) rather than using deep inheritance."),
        ("Runtime Dynamic Dispatch", "RegionalSweepEngine manages std::vector<BaseSensorNode*> base pointers. Iterative sweep cycles execute dynamic method dispatch via vtable/_vptr bindings at runtime.")
    ]
    u3_data = [[Paragraph(f"<b>{k}</b>", table_cell_style), Paragraph(v, table_cell_style)] for k, v in u3_items]
    t3 = Table([[Paragraph("Requirement", table_header_style), Paragraph("Implementation & Technical Role", table_header_style)]] + u3_data, colWidths=[150, 350])
    t3.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), c_primary),
        ('BOX', (0,0), (-1,-1), 1, c_border),
        ('INNERGRID', (0,0), (-1,-1), 0.5, c_border),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
        ('LEFTPADDING', (0,0), (-1,-1), 6),
        ('RIGHTPADDING', (0,0), (-1,-1), 6),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, c_light_bg]),
    ]))
    story.append(t3)
    story.append(Spacer(1, 10))

    # --- Unit IV ---
    story.append(Paragraph("3.4 Unit IV: Streams, Persistence, Exception Safety & STL", h2_style))
    u4_items = [
        ("Custom Exception Hierarchy", "SensorFailureException, TelemetryChecksumMismatch, and ThresholdOverflowException inherit from std::exception and provide diagnostic what() overrides."),
        ("Persistent Binary Logging", "IncidentLogger serializes POD IncidentRecord structs using binary std::fstream (std::ios::binary | std::ios::in | std::ios::out)."),
        ("Transactional Rollbacks", "beginTransaction() records commit offsets. If corrupt records occur, rollback() reposition file pointers using seekp() and seekg() to mark invalid entries atomically."),
        ("STL Containers & Algorithms", "std::vector and std::map store nodes and hazard registries. std::sort and std::for_each perform ranking and pipeline dispatching."),
        ("Custom Functors", "HazardPriorityComparator (descending hazard sort) and AlertDispatcher (conditional route filtering) provide functional dispatch objects."),
        ("Custom Stream Manipulators", "alertHeader, alertFooter, and parameterized ColFmt encapsulate tabular column formatting for clean console reporting.")
    ]
    u4_data = [[Paragraph(f"<b>{k}</b>", table_cell_style), Paragraph(v, table_cell_style)] for k, v in u4_items]
    t4 = Table([[Paragraph("Requirement", table_header_style), Paragraph("Implementation & Technical Role", table_header_style)]] + u4_data, colWidths=[150, 350])
    t4.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), c_secondary),
        ('BOX', (0,0), (-1,-1), 1, c_border),
        ('INNERGRID', (0,0), (-1,-1), 0.5, c_border),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
        ('LEFTPADDING', (0,0), (-1,-1), 6),
        ('RIGHTPADDING', (0,0), (-1,-1), 6),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, c_light_bg]),
    ]))
    story.append(t4)

    # =========================================================================
    # SECTION 4: EXECUTION TRACE & VERIFICATION
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("4. Verification & Live Execution Output", h1_style))
    story.append(Paragraph(
        "The compiled binary (<code>alf_ewg.exe</code>) was executed on the target environment. The entire lifecycle—from station booting to exception handling, polymorphic sweep, binary persistence, and evacuation dispatch—executed synchronously with exit code 0.",
        body_style
    ))

    # Console output snippet box
    output_snippet = """
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
--- Hazard Index Calculations ---
  computeHazardIndex(rainfall=85, slope=42)          = 67.800
  computeHazardIndex(soil=78, vibr=5.5, river=3.2)  = 33.925
  computeHazardIndex(rain=70, slope=35, sat=65, v=4) = 45.300
  Geocoord transform -> (30.318, 78.034)
  Telemetry accumulator -> 117.400

--- Calibrating Autonomous Hybrid Probes (Diamond Hierarchy) ---
  [Device AHP-KDN-01] Calibrated at (30.7340, 79.0660)
  [Device AHP-BDR-02] Calibrated at (30.7440, 79.4930)
  [Device AHP-CHM-03] Calibrated at (30.4000, 79.3500)
  [Solar AHP-KDN-01] Harvested 10.80 Wh today

--- Cyclical Sensor Sweep (Polymorphic vtable Dispatch) ---
  Node[101] HazardIdx=51.90 -> ORANGE (Warning)
  Node[102] HazardIdx=37.10 -> YELLOW (Watch)
  [LoRa @867.00MHz SF=10.00] Pkt#1: ALERT! HazardIdx=61.440000
  Node[103] HazardIdx=61.44 -> RED (Danger)  * THRESHOLD TRIGGERED

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
    """
    out_table = Table([[Paragraph(f"<font color='#EDF2F7'><pre>{output_snippet.strip()}</pre></font>", code_style)]], colWidths=[500])
    out_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#1A202C")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#2D3748")),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
        ('LEFTPADDING', (0,0), (-1,-1), 8),
        ('RIGHTPADDING', (0,0), (-1,-1), 8),
    ]))
    story.append(out_table)
    story.append(Spacer(1, 10))

    # =========================================================================
    # SECTION 5: SAFETY, MEMORY & CONCLUSION
    # =========================================================================
    story.append(Paragraph("5. Memory Management & Exception Safety Analysis", h1_style))
    story.append(Paragraph(
        "<b>RAII & Heap Integrity:</b> All dynamically allocated resources in <code>TelemetryBuffer</code> (raw <code>double*</code> buffer arrays) and <code>SensorStation</code> are managed via strict RAII protocols. Parameterized dynamic constructors allocate buffers, deep copy constructors and copy assignment operators duplicate buffer contents to isolate object lifetimes, and explicit destructors call <code>delete[]</code>, preventing dangling pointers and memory leaks upon scope termination.",
        body_style
    ))
    story.append(Paragraph(
        "<b>Exception Guarantees:</b> Custom exceptions derived from <code>std::exception</code> (such as <code>ThresholdOverflowException</code>) provide the strong exception safety guarantee. When emergency limits are breached, station states are updated safely before control transfers to the caller's catch block, allowing continuous operational monitoring.",
        body_style
    ))

    story.append(Spacer(1, 10))
    story.append(Paragraph("6. Project Submission Summary", h1_style))
    
    summary_data = [
        [Paragraph("<b>Project Option:</b>", table_cell_style), Paragraph("Option 1: Autonomous Landslide & Flash-Flood Early Warning Grid", table_cell_style)],
        [Paragraph("<b>Source File:</b>", table_cell_style), Paragraph("<code>ALF-EWG.cpp</code> (Tested & compiled with MinGW g++)", table_cell_style)],
        [Paragraph("<b>Executable Binary:</b>", table_cell_style), Paragraph("<code>ALF-EWG.exe</code> (Exit code: 0, clean termination)", table_cell_style)],
        [Paragraph("<b>Persistence Artifact:</b>", table_cell_style), Paragraph("<code>incidents.bin</code> (Validated binary stream serialization)", table_cell_style)],
        [Paragraph("<b>Academic Subject:</b>", table_cell_style), Paragraph("Object-Oriented Programming Using C++ (CIC-211)", table_cell_style)],
        [Paragraph("<b>Institution:</b>", table_cell_style), Paragraph("Maharaja Agrasen Institute of Technology (MAIT)", table_cell_style)],
    ]
    sum_table = Table(summary_data, colWidths=[150, 350])
    sum_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), c_light_bg),
        ('BOX', (0,0), (-1,-1), 1, c_secondary),
        ('INNERGRID', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E0")),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
        ('LEFTPADDING', (0,0), (-1,-1), 8),
        ('RIGHTPADDING', (0,0), (-1,-1), 8),
    ]))
    story.append(sum_table)

    # Build the document
    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"[OK] Generated report: {filename}")

if __name__ == "__main__":
    out_dir = r"C:\Users\pmajo\.gemini\antigravity\scratch\ALF-EWG"
    pdf_path = os.path.join(out_dir, "ALF-EWG_Technical_Project_Report.pdf")
    build_pdf(pdf_path)
