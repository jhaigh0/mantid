#ifndef INSTRUMENTWINDOWRENDERTAB_H_
#define INSTRUMENTWINDOWRENDERTAB_H_

#include "InstrumentWindowTab.h"

#include "MantidQtAPI/GraphOptions.h"

class BinDialog;
class ColorMapWidget;
class MantidColorMap;

class QPushButton;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QAction;
class QMenu;

/**
  * Implements the Render tab in InstrumentWindow.
  */
class InstrumentWindowRenderTab: public InstrumentWindowTab
{
  Q_OBJECT

public:
  InstrumentWindowRenderTab(InstrumentWindow* instrWindow);
  ~InstrumentWindowRenderTab();
  void initSurface();
  void saveSettings(QSettings&)const;
  void loadSettings(const QSettings&);
  void setupColorBarScaling(const MantidColorMap&,double);
  GraphOptions::ScaleType getScaleType()const;
  void setScaleType(GraphOptions::ScaleType type);
  void setAxis(const QString& axisName);
  bool areAxesOn()const;
  void setupColorBar(const MantidColorMap&,double,double,double,bool);

signals:
  void rescaleColorMap();
  void setAutoscaling(bool);

public slots:
  void setMinValue(double value, bool apply = true);
  void setMaxValue(double value, bool apply = true);
  void setRange(double minValue, double maxValue, bool apply = true);
  void showAxes(bool on);
  void displayDetectorsOnly(bool yes);
  void setColorMapAutoscaling(bool);

private slots:
  void changeColormap(const QString & filename = "");
  void showResetView(int);
  void showFlipControl(int);
  void flipUnwrappedView(bool);
  /// Called before the display setting menu opens. Filters out menu options.
  void displaySettingsAboutToshow();
  /// Change the type of the surface
  void setSurfaceType(int);
  void surfaceTypeChanged(int);
  void colorMapChanged();
  void scaleTypeChanged(int);
  void glOptionChanged(bool);

private:
  void showEvent (QShowEvent *);

  QMenu* createPeaksMenu();
  QFrame * setupAxisFrame();
  QComboBox* m_renderMode;
  QPushButton *mSaveImage;
  ColorMapWidget* m_colorMapWidget;
  QFrame* m_resetViewFrame;
  QComboBox *mAxisCombo;
  QCheckBox *m_flipCheckBox;
  QPushButton *m_peakOverlaysButton;
  QCheckBox *m_autoscaling;

  QAction *m_colorMap;
  QAction *m_backgroundColor;
  QAction *m_displayAxes;
  QAction *m_displayDetectorsOnly;
  QAction *m_wireframe;
  QAction *m_lighting;
  QAction *m_GLView; ///< toggle between OpenGL and simple view

  friend class InstrumentWindow;
  
};


#endif /*INSTRUMENTWINDOWRENDERTAB_H_*/
