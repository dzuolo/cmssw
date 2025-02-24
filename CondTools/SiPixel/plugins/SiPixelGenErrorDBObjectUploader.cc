#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>

#include "CondFormats/DataRecord/interface/SiPixelGenErrorDBObjectRcd.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelGenErrorDBObject.h"

#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/CommonTopologies/interface/PixelGeomDetUnit.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"

#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/TrackerCommon/interface/PixelBarrelName.h"
#include "DataFormats/TrackerCommon/interface/PixelEndcapName.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Utilities/interface/ESGetToken.h"

class SiPixelGenErrorDBObjectUploader : public edm::one::EDAnalyzer<> {
public:
  explicit SiPixelGenErrorDBObjectUploader(const edm::ParameterSet&);
  ~SiPixelGenErrorDBObjectUploader() override;

  typedef std::vector<std::string> vstring;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  vstring theGenErrorCalibrations;
  std::string theGenErrorBaseString;
  float theVersion;
  float theMagField;
  std::vector<uint32_t> theDetIds;
  vstring theBarrelLocations;
  vstring theEndcapLocations;
  std::vector<uint32_t> theBarrelGenErrIds;
  std::vector<uint32_t> theEndcapGenErrIds;
  bool useVectorIndices;
  edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> trackerGeometryToken_;
  edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> trackerTopologyToken_;
};

SiPixelGenErrorDBObjectUploader::SiPixelGenErrorDBObjectUploader(const edm::ParameterSet& iConfig)
    : theGenErrorCalibrations(iConfig.getParameter<vstring>("siPixelGenErrorCalibrations")),
      theGenErrorBaseString(iConfig.getParameter<std::string>("theGenErrorBaseString")),
      theVersion(iConfig.getParameter<double>("Version")),
      theMagField(iConfig.getParameter<double>("MagField")),
      theBarrelLocations(iConfig.getParameter<std::vector<std::string> >("barrelLocations")),
      theEndcapLocations(iConfig.getParameter<std::vector<std::string> >("endcapLocations")),
      theBarrelGenErrIds(iConfig.getParameter<std::vector<uint32_t> >("barrelGenErrIds")),
      theEndcapGenErrIds(iConfig.getParameter<std::vector<uint32_t> >("endcapGenErrIds")),
      useVectorIndices(iConfig.getUntrackedParameter<bool>("useVectorIndices", false)),
      trackerGeometryToken_(esConsumes()),
      trackerTopologyToken_(esConsumes()) {}

SiPixelGenErrorDBObjectUploader::~SiPixelGenErrorDBObjectUploader() = default;

void SiPixelGenErrorDBObjectUploader::beginJob() {}

void SiPixelGenErrorDBObjectUploader::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  //--- Make the POOL-ORA object to store the database object
  SiPixelGenErrorDBObject* obj = new SiPixelGenErrorDBObject;

  // Local variables
  const char* tempfile;
  int m;

  // Set the number of GenErrors to be passed to the dbobject
  obj->setNumOfTempl(theGenErrorCalibrations.size());

  // Set the version of the GenError dbobject - this is an external parameter
  obj->setVersion(theVersion);

  // Open the GenError file(s)
  for (m = 0; m < obj->numOfTempl(); ++m) {
    edm::FileInPath file(theGenErrorCalibrations[m].c_str());
    tempfile = (file.fullPath()).c_str();
    std::ifstream in_file(tempfile, std::ios::in);
    if (in_file.is_open()) {
      edm::LogInfo("GenError Info") << "Opened GenError File: " << file.fullPath().c_str();

      // Local variables
      char title_char[80], c;
      SiPixelGenErrorDBObject::char2float temp;
      float tempstore;
      int iter, j, k;

      // GenErrors contain a header char - we must be clever about storing this
      for (iter = 0; (c = in_file.get()) != '\n'; ++iter) {
        if (iter < 79) {
          title_char[iter] = c;
        }
      }
      if (iter > 78) {
        iter = 78;
      }
      title_char[iter + 1] = '\n';

      for (j = 0; j < 80; j += 4) {
        temp.c[0] = title_char[j];
        temp.c[1] = title_char[j + 1];
        temp.c[2] = title_char[j + 2];
        temp.c[3] = title_char[j + 3];
        obj->push_back(temp.f);
        obj->setMaxIndex(obj->maxIndex() + 1);
      }

      // Check if the magnetic field is the same as in the header of the input files
      for (k = 0; k < 80; k++) {
        if ((title_char[k] == '@') && (title_char[k - 1] == 'T')) {
          double localMagField = (((int)title_char[k - 4]) - 48) * 10 + ((int)title_char[k - 2]) - 48;
          if (theMagField != localMagField) {
            edm::LogPrint("SiPixelGenErrorDBObjectUploader")
                << "\n -------- WARNING -------- \n Magnetic field in the cfg is " << theMagField << "T while it is "
                << title_char[k - 4] << title_char[k - 2] << title_char[k - 1]
                << " in the header \n ------------------------- \n " << std::endl;
          }
        }
      }

      // Fill the dbobject
      in_file >> tempstore;
      while (!in_file.eof()) {
        obj->setMaxIndex(obj->maxIndex() + 1);
        obj->push_back(tempstore);
        in_file >> tempstore;
      }

      in_file.close();
    } else {
      // If file didn't open, report this
      edm::LogError("SiPixelGenErrorDBObjectUploader") << "Error opening File" << tempfile;
    }
  }

  //get TrackerGeometry from the event setup
  const edm::ESHandle<TrackerGeometry> pDD = iSetup.getHandle(trackerGeometryToken_);
  const TrackerGeometry* tGeo = &iSetup.getData(trackerGeometryToken_);

  // Use the TrackerTopology class for layer/disk etc. number
  const TrackerTopology* tTopo = &iSetup.getData(trackerTopologyToken_);

  // Check if we are using Phase-1 or Phase-2 geometry
  int phase = 0;
  if (pDD->isThere(GeomDetEnumerators::P1PXB) && pDD->isThere(GeomDetEnumerators::P1PXEC) == true) {
    phase = 1;
  } else if (pDD->isThere(GeomDetEnumerators::P2PXB) && pDD->isThere(GeomDetEnumerators::P2PXEC) == true) {
    phase = 2;
  }
  edm::LogPrint("SiPixelGenErrorDBObjectUploader") << "Phase-" << phase << " geometry is used" << std::endl;

  //Loop over the detector elements and put the GenError IDs in place
  for (const auto& it : pDD->detUnits()) {
    if (it != nullptr) {
      // Here is the actual looping step over all DetIds:
      DetId detid = it->geographicalId();
      unsigned int layer = 0, ladder = 0, disk = 0, side = 0, blade = 0, panel = 0, module = 0;
      // Some extra variables that can be used for Phase 1 - comment in if needed
      // unsigned int shl=0, sec=0, half=0, flipped=0, ring=0;
      short thisID = 10000;
      unsigned int iter;

      // Now we sort them into the Barrel and Endcap:
      //Barrel Pixels first
      if ((phase == 1 && detid.subdetId() == static_cast<int>(PixelSubdetector::PixelBarrel)) ||
          (phase == 2 && tGeo->geomDetSubDetector(detid.subdetId()) == GeomDetEnumerators::P2PXB)) {
        //Get the layer, ladder, and module corresponding to this DetID
        layer = tTopo->pxbLayer(detid.rawId());
        ladder = tTopo->pxbLadder(detid.rawId());
        module = tTopo->pxbModule(detid.rawId());

        if (useVectorIndices) {
          --layer;
          --ladder;
          --module;
        }

        //Assign template IDs
        //Loop over all the barrel locations
        for (iter = 0; iter < theBarrelLocations.size(); ++iter) {
          //get the string of this barrel location
          std::string loc_string = theBarrelLocations[iter];
          //find where the delimiters are
          unsigned int first_delim_pos = loc_string.find('_');
          unsigned int second_delim_pos = loc_string.find('_', first_delim_pos + 1);
          //get the layer, ladder, and module as unsigned ints
          unsigned int checklayer = (unsigned int)stoi(loc_string.substr(0, first_delim_pos));
          unsigned int checkladder =
              (unsigned int)stoi(loc_string.substr(first_delim_pos + 1, second_delim_pos - first_delim_pos - 1));
          unsigned int checkmodule = (unsigned int)stoi(loc_string.substr(second_delim_pos + 1, 5));
          //check them against the desired layer, ladder, and module
          if (ladder == checkladder && layer == checklayer && module == checkmodule)
            //if they match, set the template ID
            thisID = (short)theBarrelGenErrIds[iter];
        }

        if (thisID == 10000 || (!(*obj).putGenErrorID(detid.rawId(), thisID)))
          edm::LogPrint("SiPixelGenErrorDBObjectUploader")
              << " Could not fill barrel layer " << layer << ", module " << module << "\n";
        edm::LogPrint("SiPixelGenErrorDBObjectUploader")
            << "This is a barrel element with: layer " << layer << ", ladder " << ladder << " and module " << module;
      }
      //Now endcaps
      else if ((phase == 1 && detid.subdetId() == static_cast<int>(PixelSubdetector::PixelEndcap)) ||
               (phase == 2 && tGeo->geomDetSubDetector(detid.subdetId()) == GeomDetEnumerators::P2PXEC)) {
        //Get the DetID's disk, blade, side, panel, and module
        disk = tTopo->pxfDisk(detid.rawId());    //1,2,3
        blade = tTopo->pxfBlade(detid.rawId());  //1-56 (Ring 1 is 1-22, Ring 2 is 23-56)
        side = tTopo->pxfSide(detid.rawId());    //side=1 for -z, 2 for +z
        panel = tTopo->pxfPanel(detid.rawId());  //panel=1,2

        if (useVectorIndices) {
          --disk;
          --blade;
          --side;
          --panel;
        }

        //Assign IDs
        //Loop over all the endcap locations
        for (iter = 0; iter < theEndcapLocations.size(); ++iter) {
          //get the string of this barrel location
          std::string loc_string = theEndcapLocations[iter];
          //find where the delimiters are
          unsigned int first_delim_pos = loc_string.find('_');
          unsigned int second_delim_pos = loc_string.find('_', first_delim_pos + 1);
          unsigned int third_delim_pos = loc_string.find('_', second_delim_pos + 1);
          //get the disk, blade, side, panel, and module as unsigned ints
          unsigned int checkdisk = (unsigned int)stoi(loc_string.substr(0, first_delim_pos));
          unsigned int checkblade =
              (unsigned int)stoi(loc_string.substr(first_delim_pos + 1, second_delim_pos - first_delim_pos - 1));
          unsigned int checkside =
              (unsigned int)stoi(loc_string.substr(second_delim_pos + 1, third_delim_pos - second_delim_pos - 1));
          unsigned int checkpanel = (unsigned int)stoi(loc_string.substr(third_delim_pos + 1, 5));
          //check them against the desired disk, blade, side, panel, and module
          if (disk == checkdisk && blade == checkblade && side == checkside && panel == checkpanel)
            //if they match, set the template ID
            thisID = (short)theEndcapGenErrIds[iter];
        }

        if (thisID == 10000 || (!(*obj).putGenErrorID(detid.rawId(), thisID)))
          edm::LogPrint("SiPixelGenErrorDBObjectUploader")
              << " Could not fill endcap det unit" << side << ", disk " << disk << ", blade " << blade << ", panel "
              << panel << ".\n";
        edm::LogPrint("SiPixelGenErrorDBObjectUploader") << "This is an endcap element with: side " << side << ", disk "
                                                         << disk << ", blade " << blade << ", panel " << panel;
      } else {
        continue;
      }

      //Print out the assignment of this DetID
      short mapnum;
      mapnum = (*obj).getGenErrorID(detid.rawId());
      edm::LogPrint("SiPixelGenErrorDBObjectUploader")
          << "The DetID: " << detid.rawId() << " is mapped to the template: " << mapnum << "\n";
    }
  }

  //--- Create a new IOV
  edm::Service<cond::service::PoolDBOutputService> poolDbService;
  if (!poolDbService.isAvailable())  // Die if not available
    throw cms::Exception("NotAvailable") << "PoolDBOutputService not available";
  if (poolDbService->isNewTagRequest("SiPixelGenErrorDBObjectRcd"))
    poolDbService->writeOne(obj, poolDbService->beginOfTime(), "SiPixelGenErrorDBObjectRcd");
  else
    poolDbService->writeOne(obj, poolDbService->currentTime(), "SiPixelGenErrorDBObjectRcd");
}

void SiPixelGenErrorDBObjectUploader::endJob() {}

DEFINE_FWK_MODULE(SiPixelGenErrorDBObjectUploader);
