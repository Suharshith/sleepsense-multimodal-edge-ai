/*
 * Adaptive Multimodal Edge AI Sleep Apnea System V2
 * Frontend Dashboard Application Script
 */

document.addEventListener('DOMContentLoaded', () => {
  // Global State
  let activeSessionId = "SESSION-DEMO-001";
  let isMonitoringActive = true;
  let chartPhysio = null;
  let chartRisk = null;

  // DOM Elements
  const audioClassEl = document.getElementById('audioClassDisplay');
  const audioConfEl = document.getElementById('audioConfidenceDisplay');
  const audioDbEl = document.getElementById('audioDbDisplay');

  const spo2ValEl = document.getElementById('spo2ValueDisplay');
  const spo2AlertEl = document.getElementById('spo2AlertBadge');
  const signalQualEl = document.getElementById('signalQualityDisplay');

  const hrValEl = document.getElementById('hrValueDisplay');
  const fingerStatusEl = document.getElementById('fingerStatusDisplay');

  const postureEl = document.getElementById('postureDisplay');
  const movementValEl = document.getElementById('movementValueDisplay');
  const restlessValEl = document.getElementById('restlessValueDisplay');

  const riskScoreEl = document.getElementById('riskScoreDisplay');
  const riskLevelEl = document.getElementById('riskLevelBadge');

  const cntSnoringEl = document.getElementById('cntSnoring');
  const cntAbnormalEl = document.getElementById('cntAbnormal');
  const cntDesatEl = document.getElementById('cntDesat');

  const barAudio = document.getElementById('barAudio');
  const barSpo2 = document.getElementById('barSpo2');
  const barHr = document.getElementById('barHr');
  const barMotion = document.getElementById('barMotion');

  const explainAudioPct = document.getElementById('explainAudioPct');
  const explainSpo2Pct = document.getElementById('explainSpo2Pct');
  const explainHrPct = document.getElementById('explainHrPct');
  const explainMotionPct = document.getElementById('explainMotionPct');

  const eventsTableBody = document.getElementById('eventsTableBody');

  // Modals & Buttons
  const benchmarkModal = document.getElementById('benchmarkModal');
  const btnBenchmark = document.getElementById('btnBenchmark');
  const btnCloseBenchmark = document.getElementById('btnCloseBenchmark');

  const reportModal = document.getElementById('reportModal');
  const btnGenerateReport = document.getElementById('btnGenerateReport');
  const btnCloseReport = document.getElementById('btnCloseReport');
  const btnPrintReport = document.getElementById('btnPrintReport');
  const btnToggleSession = document.getElementById('btnToggleSession');

  // Initialize Charts
  initCharts();

  // Initial Fetch & Start Polling Interval (Every 2 Seconds)
  fetchData();
  fetchModelBenchmark();
  setInterval(fetchData, 2000);

  // ----------------------------------------------------
  // Chart Initialization Function
  // ----------------------------------------------------
  function initCharts() {
    const ctxPhysio = document.getElementById('physioChart').getContext('2d');
    chartPhysio = new Chart(ctxPhysio, {
      type: 'line',
      data: {
        labels: [],
        datasets: [
          {
            label: 'SpO2 (%)',
            data: [],
            borderColor: '#f43f5e',
            backgroundColor: 'rgba(244, 63, 94, 0.1)',
            borderWidth: 2,
            tension: 0.3,
            fill: true,
            yAxisID: 'ySpo2'
          },
          {
            label: 'Heart Rate (BPM)',
            data: [],
            borderColor: '#06b6d4',
            borderWidth: 2,
            tension: 0.3,
            fill: false,
            yAxisID: 'yHr'
          }
        ]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        scales: {
          x: { ticks: { color: '#64748b', font: { size: 10 } }, grid: { color: '#1e293b' } },
          ySpo2: {
            type: 'linear', position: 'left', min: 75, max: 100,
            ticks: { color: '#f43f5e', font: { size: 10 } }, grid: { color: '#1e293b' }
          },
          yHr: {
            type: 'linear', position: 'right', min: 40, max: 130,
            ticks: { color: '#06b6d4', font: { size: 10 } }, grid: { drawOnChartArea: false }
          }
        },
        plugins: { legend: { labels: { color: '#94a3b8', font: { size: 11 } } } }
      }
    });

    const ctxRisk = document.getElementById('riskChart').getContext('2d');
    chartRisk = new Chart(ctxRisk, {
      type: 'line',
      data: {
        labels: [],
        datasets: [
          {
            label: 'Risk Score (0-100)',
            data: [],
            borderColor: '#f59e0b',
            backgroundColor: 'rgba(245, 158, 11, 0.15)',
            borderWidth: 2.5,
            tension: 0.3,
            fill: true,
            yAxisID: 'yRisk'
          },
          {
            label: 'Sound Level (dB SPL)',
            data: [],
            borderColor: '#8b5cf6',
            borderWidth: 1.5,
            borderDash: [4, 4],
            tension: 0.2,
            fill: false,
            yAxisID: 'yDb'
          }
        ]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        scales: {
          x: { ticks: { color: '#64748b', font: { size: 10 } }, grid: { color: '#1e293b' } },
          yRisk: {
            type: 'linear', position: 'left', min: 0, max: 100,
            ticks: { color: '#f59e0b', font: { size: 10 } }, grid: { color: '#1e293b' }
          },
          yDb: {
            type: 'linear', position: 'right', min: 30, max: 90,
            ticks: { color: '#8b5cf6', font: { size: 10 } }, grid: { drawOnChartArea: false }
          }
        },
        plugins: { legend: { labels: { color: '#94a3b8', font: { size: 11 } } } }
      }
    });
  }

  // ----------------------------------------------------
  // Primary Data Fetch & UI Update Loop
  // ----------------------------------------------------
  async function fetchData() {
    if (!isMonitoringActive) return;

    try {
      // 1. Fetch Latest Telemetry Reading
      const resLatest = await fetch('/api/readings/latest');
      if (resLatest.ok) {
        const r = await resLatest.json();
        if (r) updateLatestUI(r);
      }

      // 2. Fetch Session Readings for Timeline Charts
      const resReadings = await fetch(`/api/readings/${activeSessionId}`);
      if (resReadings.ok) {
        const readings = await resReadings.json();
        updateCharts(readings);
      }

      // 3. Fetch Event Logs
      const resEvents = await fetch(`/api/events/${activeSessionId}`);
      if (resEvents.ok) {
        const events = await resEvents.json();
        updateEventsTable(events);
      }
    } catch (err) {
      console.warn("Polling endpoint error:", err);
    }
  }

  // ----------------------------------------------------
  // Update Live Telemetry Cards & Risk Factor Meters
  // ----------------------------------------------------
  function updateLatestUI(r) {
    // Audio Card
    audioClassEl.textContent = r.audio.class;
    audioConfEl.textContent = `${Math.round(r.audio.confidence * 100)}%`;
    audioDbEl.textContent = `${r.audio.dbSpl} dB`;

    // SpO2 Card
    spo2ValEl.innerHTML = `${r.physio.spo2}<span class="unit">%</span>`;
    if (r.physio.spo2 < 92.0 && r.physio.spo2 > 0) {
      spo2AlertEl.textContent = "DESAT ALERT";
      spo2AlertEl.className = "badge-alert text-danger";
      spo2AlertEl.style.background = "rgba(244, 63, 94, 0.2)";
    } else {
      spo2AlertEl.textContent = "NORMAL";
      spo2AlertEl.className = "badge-alert";
      spo2AlertEl.style.background = "rgba(16, 185, 129, 0.2)";
    }
    signalQualEl.textContent = `${Math.round(r.physio.signalQuality * 100)}%`;

    // HR Card
    hrValEl.innerHTML = `${Math.round(r.physio.heartRate)}<span class="unit">BPM</span>`;
    fingerStatusEl.textContent = r.physio.fingerDetected ? "Detected" : "No Finger";
    fingerStatusEl.className = r.physio.fingerDetected ? "sub-value text-success" : "sub-value text-danger";

    // Motion Card
    postureEl.textContent = r.motion.posture;
    movementValEl.textContent = `${r.motion.movement} g`;
    restlessValEl.textContent = `${(r.motion.movement * 0.2).toFixed(2)}`;

    // Risk Score Card
    riskScoreEl.textContent = r.risk.score;
    riskLevelEl.textContent = `${r.risk.level} RISK`;

    if (r.risk.level === "ELEVATED") {
      riskLevelEl.className = "risk-level-badge badge-elevated";
      riskScoreEl.style.color = "var(--accent-rose)";
    } else if (r.risk.level === "MODERATE") {
      riskLevelEl.className = "risk-level-badge badge-moderate";
      riskScoreEl.style.color = "var(--accent-amber)";
    } else {
      riskLevelEl.className = "risk-level-badge badge-low";
      riskScoreEl.style.color = "var(--accent-emerald)";
    }

    // Risk Factor Explainability Progress Bars
    const audioP = Math.round(r.risk.audioContrib || (r.audio.class !== "Normal Breathing" ? 75 : 15));
    const spo2P = Math.round(r.risk.spo2Contrib || (r.physio.spo2 < 92 ? (92 - r.physio.spo2) * 12 : 10));
    const hrP = Math.round(r.risk.hrContrib || (r.physio.heartRate > 90 ? 40 : 10));
    const motionP = Math.round(r.risk.motionContrib || 15);

    barAudio.style.width = `${audioP}%`;
    barSpo2.style.width = `${spo2P}%`;
    barHr.style.width = `${hrP}%`;
    barMotion.style.width = `${motionP}%`;

    explainAudioPct.textContent = `${audioP}%`;
    explainSpo2Pct.textContent = `${spo2P}%`;
    explainHrPct.textContent = `${hrP}%`;
    explainMotionPct.textContent = `${motionP}%`;
  }

  // ----------------------------------------------------
  // Update Real-Time Timeline Charts
  // ----------------------------------------------------
  function updateCharts(readings) {
    if (!readings || readings.length === 0) return;

    const labels = readings.map(r => {
      const d = new Date(r.timestamp);
      return `${d.getHours().toString().padStart(2, '0')}:${d.getMinutes().toString().padStart(2, '0')}:${d.getSeconds().toString().padStart(2, '0')}`;
    });

    const spo2Data = readings.map(r => r.physio.spo2);
    const hrData = readings.map(r => r.physio.heartRate);
    const riskData = readings.map(r => r.risk.score);
    const dbData = readings.map(r => r.audio.dbSpl);

    // Update Physio Chart
    chartPhysio.data.labels = labels;
    chartPhysio.data.datasets[0].data = spo2Data;
    chartPhysio.data.datasets[1].data = hrData;
    chartPhysio.update('none');

    // Update Risk Chart
    chartRisk.data.labels = labels;
    chartRisk.data.datasets[0].data = riskData;
    chartRisk.data.datasets[1].data = dbData;
    chartRisk.update('none');
  }

  // ----------------------------------------------------
  // Update Validated Event Log Table & Counters
  // ----------------------------------------------------
  function updateEventsTable(events) {
    if (!events) return;

    let snoringCount = 0;
    let abnormalCount = 0;
    let desatCount = 0;

    eventsTableBody.innerHTML = '';
    const recentEvents = events.slice(-15).reverse();

    recentEvents.forEach(evt => {
      if (evt.eventType === "SNORING") snoringCount++;
      else if (evt.eventType === "ABNORMAL_BREATHING") abnormalCount++;
      else if (evt.eventType === "OXYGEN_DESATURATION") desatCount++;

      const tr = document.createElement('tr');
      const timeStr = new Date(evt.timestamp).toLocaleTimeString();

      let typeBadge = '';
      if (evt.eventType === "SNORING") {
        typeBadge = '<span class="badge-version" style="background:rgba(139,92,246,0.2);color:#8b5cf6;border-color:#8b5cf6;">SNORING</span>';
      } else if (evt.eventType === "ABNORMAL_BREATHING") {
        typeBadge = '<span class="badge-version" style="background:rgba(245,158,11,0.2);color:#f59e0b;border-color:#f59e0b;">ABNORMAL BREATHING</span>';
      } else {
        typeBadge = '<span class="badge-version" style="background:rgba(244,63,94,0.2);color:#f43f5e;border-color:#f43f5e;">DESAT &lt;92%</span>';
      }

      const ctx = evt.sensorContext || {};
      const ctxText = `SpO2: ${ctx.spo2 || '--'}% | HR: ${ctx.heartRate || '--'} BPM | ${ctx.dbSpl ? ctx.dbSpl + ' dB' : ''} | ${ctx.posture || 'Supine'}`;

      tr.innerHTML = `
        <td>${timeStr}</td>
        <td>${typeBadge}</td>
        <td>${Math.round((evt.confidence || 0.95) * 100)}%</td>
        <td>${evt.durationSeconds || 20}s</td>
        <td style="color:#94a3b8;font-size:12px;">${ctxText}</td>
      `;
      eventsTableBody.appendChild(tr);
    });

    cntSnoringEl.textContent = events.filter(e => e.eventType === "SNORING").length;
    cntAbnormalEl.textContent = events.filter(e => e.eventType === "ABNORMAL_BREATHING").length;
    cntDesatEl.textContent = events.filter(e => e.eventType === "OXYGEN_DESATURATION").length;
  }

  // ----------------------------------------------------
  // Fetch Model Benchmark Data
  // ----------------------------------------------------
  async function fetchModelBenchmark() {
    try {
      const res = await fetch('/api/model/performance');
      if (res.ok) {
        const data = await res.json();
        if (data.originalModel && data.tinyMLModel) {
          document.getElementById('bmFloatAcc').textContent = `${(data.originalModel.accuracy * 100).toFixed(1)}%`;
          document.getElementById('bmInt8Acc').textContent = `${(data.tinyMLModel.accuracy * 100).toFixed(1)}%`;
          document.getElementById('bmFloatF1').textContent = data.originalModel.f1Score.toFixed(3);
          document.getElementById('bmInt8F1').textContent = data.tinyMLModel.f1Score.toFixed(3);
          document.getElementById('bmAccLoss').textContent = `-${data.comparison.accuracyLossPct}%`;
          document.getElementById('bmCompRatio').textContent = `${data.comparison.sizeReductionRatio}x Reduction`;
          document.getElementById('bmSpeedup').textContent = `${data.comparison.latencySpeedup}x Speedup`;
        }
      }
    } catch (e) {
      console.log("Benchmark fetch note:", e);
    }
  }

  // ----------------------------------------------------
  // Fetch & Populate Printable Sleep Report Modal
  // ----------------------------------------------------
  async function generateReport() {
    try {
      const res = await fetch(`/api/report/${activeSessionId}`);
      if (res.ok) {
        const rpt = await res.json();
        document.getElementById('rptPatientName').textContent = rpt.patient.name;
        document.getElementById('rptSessionId').textContent = rpt.session.sessionId;
        document.getElementById('rptDuration').textContent = `${rpt.metrics.monitoringDurationMinutes} Minutes`;
        document.getElementById('rptAvgSpo2').textContent = `${rpt.metrics.averageSpo2}%`;
        document.getElementById('rptMinSpo2').textContent = `${rpt.metrics.minimumSpo2}%`;
        document.getElementById('rptAvgHr').textContent = `${rpt.metrics.averageHeartRate} BPM`;
        document.getElementById('rptSnoreCnt').textContent = rpt.metrics.snoringEvents;
        document.getElementById('rptAbnormalCnt').textContent = rpt.metrics.abnormalBreathingEvents;
        document.getElementById('rptDesatCnt').textContent = rpt.metrics.desaturationEvents;

        document.getElementById('rptRiskLevel').textContent = rpt.riskAssessment.overallRiskLevel;
        document.getElementById('rptRiskScore').textContent = rpt.riskAssessment.overallRiskScore;

        const factorsList = document.getElementById('rptFactorsList');
        factorsList.innerHTML = '';
        rpt.riskAssessment.contributingFactors.forEach(f => {
          const li = document.createElement('li');
          li.textContent = f;
          factorsList.appendChild(li);
        });

        const recoList = document.getElementById('rptRecommendations');
        recoList.innerHTML = '';
        rpt.recommendations.forEach(rec => {
          const li = document.createElement('li');
          li.textContent = rec;
          recoList.appendChild(li);
        });

        reportModal.classList.add('active');
      }
    } catch (e) {
      console.error("Report generation failed:", e);
    }
  }

  // Event Listeners
  btnBenchmark.addEventListener('click', () => benchmarkModal.classList.add('active'));
  btnCloseBenchmark.addEventListener('click', () => benchmarkModal.classList.remove('active'));

  btnGenerateReport.addEventListener('click', generateReport);
  btnCloseReport.addEventListener('click', () => reportModal.classList.remove('active'));
  btnPrintReport.addEventListener('click', () => window.print());

  btnToggleSession.addEventListener('click', () => {
    isMonitoringActive = !isMonitoringActive;
    if (isMonitoringActive) {
      btnToggleSession.innerHTML = '<i class="fa-solid fa-stop"></i> End Session';
      document.getElementById('sessionStatusText').textContent = "SESSION ACTIVE";
    } else {
      btnToggleSession.innerHTML = '<i class="fa-solid fa-play"></i> Resume Session';
      document.getElementById('sessionStatusText').textContent = "SESSION PAUSED";
    }
  });

  // Close modals on clicking backdrop
  window.addEventListener('click', (e) => {
    if (e.target === benchmarkModal) benchmarkModal.classList.remove('active');
    if (e.target === reportModal) reportModal.classList.remove('active');
  });
});
