#include "rtl_sdr_control_panel.h"

RtlSdrControl::RtlSdrControl(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void RtlSdrControl::setupUI() {
    // Layout chính
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Nhãn tiêu đề
    QLabel *titleLabel = new QLabel("RTL-SDR Control", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Ô nhập tần số
    QLabel *frequencyLabel = new QLabel("Frequency (Hz):", this);
    frequencySpinBox = new QSpinBox(this);
    frequencySpinBox->setRange(1000000, 3000000000); // 1 MHz đến 3 GHz
    frequencySpinBox->setSingleStep(1000000);        // Bước nhảy 1 MHz
//    connect(frequencySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &RtlSdrControl::frequencyChanged);

    mainLayout->addWidget(frequencyLabel);
    mainLayout->addWidget(frequencySpinBox);

    // Chọn chế độ Gain Mode
    QLabel *gainModeLabel = new QLabel("Gain Mode:", this);
    gainModeComboBox = new QComboBox(this);
    gainModeComboBox->addItems({"Auto", "Manual"});
//    connect(gainModeComboBox, &QComboBox::currentTextChanged, this, &RtlSdrControl::gainModeChanged);

    mainLayout->addWidget(gainModeLabel);
    mainLayout->addWidget(gainModeComboBox);

    // Nút điều khiển Start/Stop
    startButton = new QPushButton("Start", this);
    stopButton = new QPushButton("Stop", this);

//    connect(startButton, &QPushButton::clicked, this, &RtlSdrControl::startDevice);
//    connect(stopButton, &QPushButton::clicked, this, &RtlSdrControl::stopDevice);

    mainLayout->addWidget(startButton);
    mainLayout->addWidget(stopButton);
}
