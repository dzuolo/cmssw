#ifndef SimG4Core_GeometryProducer_H
#define SimG4Core_GeometryProducer_H

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/one/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "SimG4Core/Notification/interface/SimActivityRegistry.h"
#include "SimG4Core/SensitiveDetector/interface/SensitiveCaloDetector.h"
#include "SimG4Core/SensitiveDetector/interface/SensitiveDetector.h"
#include "SimG4Core/SensitiveDetector/interface/SensitiveTkDetector.h"

#include "DetectorDescription/Core/interface/DDCompactView.h"
#include "DetectorDescription/DDCMS/interface/DDCompactView.h"

#include "Geometry/Records/interface/IdealGeometryRecord.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include <memory>
#include <unordered_map>

namespace sim {
  class FieldBuilder;
}

namespace cms {
  class DDCompactView;
}

class SimWatcher;
class SimProducer;
class DDDWorld;
class G4RunManagerKernel;
class SimTrackManager;
class SensitiveDetectorMakerBase;
class DDCompactView;

class GeometryProducer : public edm::one::EDProducer<edm::one::SharedResources, edm::one::WatchRuns> {
public:
  typedef std::vector<std::shared_ptr<SimProducer>> Producers;

  explicit GeometryProducer(edm::ParameterSet const &p);
  ~GeometryProducer() override;
  void beginRun(const edm::Run &r, const edm::EventSetup &c) override;
  void endRun(const edm::Run &r, const edm::EventSetup &c) override;
  void produce(edm::Event &e, const edm::EventSetup &c) override;
  void beginLuminosityBlock(edm::LuminosityBlock &, edm::EventSetup const &);

  std::vector<std::shared_ptr<SimProducer>> producers() const { return m_producers; }
  std::vector<SensitiveTkDetector *> &sensTkDetectors() { return m_sensTkDets; }
  std::vector<SensitiveCaloDetector *> &sensCaloDetectors() { return m_sensCaloDets; }

private:
  void updateMagneticField(edm::EventSetup const &es);

  G4RunManagerKernel *m_kernel;
  edm::ParameterSet m_pField;
  SimActivityRegistry m_registry;
  std::vector<std::shared_ptr<SimWatcher>> m_watchers;
  std::vector<std::shared_ptr<SimProducer>> m_producers;
  std::unique_ptr<sim::FieldBuilder> m_fieldBuilder;
  std::unique_ptr<SimTrackManager> m_trackManager;
  std::vector<SensitiveTkDetector *> m_sensTkDets;
  std::vector<SensitiveCaloDetector *> m_sensCaloDets;
  std::unordered_map<std::string, std::unique_ptr<SensitiveDetectorMakerBase>> m_sdMakers;
  edm::ParameterSet m_p;

  mutable const DDCompactView *m_pDD;
  mutable const cms::DDCompactView *m_pDD4hep;

  bool m_firstRun;
  bool m_pUseMagneticField;
  bool m_pUseSensitiveDetectors;
  bool m_pGeoFromDD4hep;

  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> tokMF_;
  edm::ESGetToken<DDCompactView, IdealGeometryRecord> tokDDD_;
  edm::ESGetToken<cms::DDCompactView, IdealGeometryRecord> tokDD4Hep_;
};

#endif
