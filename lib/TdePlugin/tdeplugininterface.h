//
// Created by victor on 9/10/23.
//

#ifndef LIBTDESKTOPENVIRONMENT_TDEPLUGININTERFACE_H
#define LIBTDESKTOPENVIRONMENT_TDEPLUGININTERFACE_H

#include <QObject>

class WmBackend;
class ScreenBackend;
class GestureBackend;
class TdePluginInterface {
public:
  virtual ~TdePluginInterface() {}

  virtual void activate() = 0;
  virtual void deactivate() = 0;

  virtual bool supportedOnThisPlatform() = 0;

  virtual WmBackend* wmBackend() = 0;
  virtual ScreenBackend* screenBackend() = 0;
  virtual GestureBackend* gestureBackend() = 0;
};

#define TdePluginInterface_iid "com.vicr123.libtdesktopenvironment.TdePluginInterface/1.0"
Q_DECLARE_INTERFACE(TdePluginInterface, TdePluginInterface_iid);

#endif // LIBTDESKTOPENVIRONMENT_TDEPLUGININTERFACE_H
