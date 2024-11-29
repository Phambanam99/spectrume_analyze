#ifndef RTL_SDR_CONTROL_H
#define RTL_SDR_CONTROL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>

class RtlSdrControl : public QWidget {
    Q_OBJECT

public:
    explicit RtlSdrControl(QWidget *parent = nullptr);

//signals:
//    void startDevice();
//    void stopDevice();
//    void frequencyChanged(int frequency);
//    void gainModeChanged(const QString &gainMode);

private:
    QSpinBox *frequencySpinBox;    // Ô nhập tần số
    QComboBox *gainModeComboBox;  // Chọn chế độ tăng cường tín hiệu
    QPushButton *startButton;     // Nút bắt đầu
    QPushButton *stopButton;      // Nút dừng

    void setupUI();               // Hàm khởi tạo giao diện
};

#endif // RTL_SDR_CONTROL_H
