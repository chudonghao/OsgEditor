//
// Created by chudonghao on 2020/7/3.
//

#ifndef OSGEDITOR_OSGDATAEDIT_H
#define OSGEDITOR_OSGDATAEDIT_H

#include <QDebug>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QWidget>
#include <type_traits>

class OsgDataEditBase : public QWidget {
  Q_OBJECT;

 protected:
  using QWidget::QWidget;

 public:
  Q_SIGNAL void valueChanged();
};

template <typename T_>
class IsVecx {
 public:
  template <typename U_>
  static typename std::conditional<std::is_enum<decltype(U_::num_components)>::value, std::true_type, std::false_type>::type check(int);

  template <typename>
  static std::false_type check(...);

  constexpr static bool value = decltype(check<T_>(0))::type::value;
};

template <typename T>
using IsMatrix = typename std::conditional<std::is_same<osg::Matrixf, T>::value || std::is_same<osg::Matrixd, T>::value, std::true_type, std::false_type>::type;

template <typename T>
class OsgDataEditFloat : public QDoubleSpinBox {
 protected:
  using Super = QDoubleSpinBox;
  using Self = OsgDataEditFloat<T>;

 public:
  using ValueType = T;

  explicit OsgDataEditFloat(QWidget *parent) {
    setRange(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max());
    setMaximumWidth(100);
  }

  ValueType getValue() const { return static_cast<ValueType>(value()); }
};

template <typename T>
class OsgDataEditVecx : public OsgDataEditBase {
 public:
  using ValueType = T;
  using BaseValueType = typename T::value_type;

  explicit OsgDataEditVecx(QWidget *parent) {
    m_spinBoxList.resize(num_components);

    auto layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    for (int i = 0; i < num_components; ++i) {
      auto spinBox = new QDoubleSpinBox(this);
      spinBox->setRange(std::numeric_limits<BaseValueType>::lowest(), std::numeric_limits<BaseValueType>::max());
      spinBox->setMaximumWidth(100);
      connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v) { setValue(i, v); });
      layout->addWidget(spinBox);
      m_spinBoxList[i] = spinBox;
    }
  }

  void setValue(ValueType value) {
    if (m_value == value) {
      return;
    }
    for (int i = 0; i < num_components; ++i) {
      m_spinBoxList[i]->blockSignals(true);
      m_spinBoxList[i]->setValue(value[i]);
      m_spinBoxList[i]->blockSignals(false);
    }
    m_value = value;
    emit valueChanged();
  }

  const ValueType &getValue() const { return m_value; }

 private:
  constexpr static int num_components = T::num_components;
  using Super = OsgDataEditBase;
  using Self = OsgDataEditVecx<T>;

  void setValue(int i, BaseValueType v) {
    m_value[i] = v;
    emit valueChanged();
    qDebug() << i << v;
  }

  ValueType m_value;
  std::vector<QDoubleSpinBox *> m_spinBoxList;
};

template <typename T>
class OsgDataEditMatrix : public OsgDataEditBase {
 protected:
  using Super = OsgDataEditBase;
  using Self = OsgDataEditMatrix<T>;

 public:
  using ValueType = T;
  using BaseValueType = typename ValueType::value_type;

  explicit OsgDataEditMatrix(QWidget *parent) {
    auto layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    for (int row = 0; row < 4; ++row) {
      for (int column = 0; column < 4; ++column) {
        auto spinBox = new QDoubleSpinBox(this);
        spinBox->setRange(std::numeric_limits<BaseValueType>::lowest(), std::numeric_limits<BaseValueType>::max());
        spinBox->setMaximumWidth(100);
        connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double v) { setValue(row, column, v); });
        layout->addWidget(spinBox, row, column);
        m_spinBoxList[row][column] = spinBox;
      }
    }
  }

  void setValue(ValueType value) {
    if (m_value == value) {
      return;
    }
    for (int row = 0; row < 4; ++row) {
      for (int column = 0; column < 4; ++column) {
        m_spinBoxList[row][column]->blockSignals(true);
        m_spinBoxList[row][column]->setValue(value(row, column));
        m_spinBoxList[row][column]->blockSignals(false);
      }
    }
    m_value = value;
    emit valueChanged();
  }

  const ValueType &getValue() const { return m_value; }

 private:
  void setValue(int row, int column, BaseValueType v) {
    m_value(row, column) = v;
    emit valueChanged();
  }
  ValueType m_value;
  QDoubleSpinBox *m_spinBoxList[4][4];
};

template <typename T>
struct DecodeOsgDataEditType {
  constexpr static auto getType() {
    if constexpr (std::is_floating_point<T>::value) {
      return (OsgDataEditFloat<T> *)nullptr;
    } else if constexpr (IsVecx<T>::value) {
      return (OsgDataEditVecx<T> *)nullptr;
    } else if constexpr (IsMatrix<T>::value) {
      return (OsgDataEditMatrix<T> *)nullptr;
    } else {
      return (void)0;
    }
  }

  using OsgDataEdit = typename std::remove_pointer<decltype(getType())>::type;
};

#endif  // OSGEDITOR_OSGDATAEDIT_H
