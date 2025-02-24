#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>

#include "CondFormats/DataRecord/interface/SiPixelTemplateDBObjectRcd.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelTemplateDBObject.h"

#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/Records/interface/IdealGeometryRecord.h"
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

class SiPixelTemplateDBObjectUploader : public edm::one::EDAnalyzer<> {
public:
  explicit SiPixelTemplateDBObjectUploader(const edm::ParameterSet&);
  ~SiPixelTemplateDBObjectUploader() override;

  typedef std::vector<std::string> vstring;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  vstring theTemplateCalibrations;
  std::string theTemplateBaseString;
  float theVersion;
  float theMagField;
  std::vector<uint32_t> theDetIds;
  vstring theBarrelLocations;
  vstring theEndcapLocations;
  std::vector<uint32_t> theBarrelTemplateIds;
  std::vector<uint32_t> theEndcapTemplateIds;
  bool useVectorIndices;
  edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> trackerGeometryToken_;
  edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> trackerTopologyToken_;
};

SiPixelTemplateDBObjectUploader::SiPixelTemplateDBObjectUploader(const edm::ParameterSet& iConfig)
    : theTemplateCalibrations(iConfig.getParameter<vstring>("siPixelTemplateCalibrations")),
      theTemplateBaseString(iConfig.getParameter<std::string>("theTemplateBaseString")),
      theVersion(iConfig.getParameter<double>("Version")),
      theMagField(iConfig.getParameter<double>("MagField")),
      theBarrelLocations(iConfig.getParameter<std::vector<std::string> >("barrelLocations")),
      theEndcapLocations(iConfig.getParameter<std::vector<std::string> >("endcapLocations")),
      theBarrelTemplateIds(iConfig.getParameter<std::vector<uint32_t> >("barrelTemplateIds")),
      theEndcapTemplateIds(iConfig.getParameter<std::vector<uint32_t> >("endcapTemplateIds")),
      useVectorIndices(iConfig.getUntrackedParameter<bool>("useVectorIndices", false)),
      trackerGeometryToken_(esConsumes()),
      trackerTopologyToken_(esConsumes()) {}

SiPixelTemplateDBObjectUploader::~SiPixelTemplateDBObjectUploader() = default;

void SiPixelTemplateDBObjectUploader::beginJob() {}

void SiPixelTemplateDBObjectUploader::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  //--- Make the POOL-ORA object to store the database object
  SiPixelTemplateDBObject obj;

  // Set the number of templates to be passed to the dbobject
  obj.setNumOfTempl(theTemplateCalibrations.size());

  // Set the version of the template dbobject - this is an external parameter
  obj.setVersion(theVersion);

  // Open the template file(s)
  for (int m = 0; m < obj.numOfTempl(); ++m) {
    edm::FileInPath file(theTemplateCalibrations[m].c_str());
    auto tempfile = (file.fullPath());
    std::ifstream in_file(tempfile.c_str(), std::ios::in);
    if (in_file.is_open()) {
      edm::LogInfo("SiPixelTemplateDBObjectUploader") << "Opened Template File: " << file.fullPath().c_str();

      // Local variables
      char title_char[80], c;
      SiPixelTemplateDBObject::char2float temp;
      float tempstore;
      int iter, j, k;

      // Templates contain a header char - we must be clever about storing this
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
        obj.push_back(temp.f);
        obj.setMaxIndex(obj.maxIndex() + 1);
      }

      // Check if the magnetic field is the same as in the header of the input files
      for (k = 0; k < 80; k++) {
        if ((title_char[k] == '@') && (title_char[k - 1] == 'T')) {
          double localMagField = (((int)title_char[k - 4]) - 48) * 10 + ((int)title_char[k - 2]) - 48;
          if (theMagField != localMagField) {
            edm::LogPrint("SiPixelTemplateDBObjectUploader")
                << "\n -------- WARNING -------- \n Magnetic field in the cfg is " << theMagField << "T while it is "
                << title_char[k - 4] << title_char[k - 2] << title_char[k - 1]
                << " in the header \n ------------------------- \n " << std::endl;
          }
        }
      }

      // Fill the dbobject
      in_file >> tempstore;
      while (!in_file.eof()) {
        obj.setMaxIndex(obj.maxIndex() + 1);
        obj.push_back(tempstore);
        in_file >> tempstore;
      }

      in_file.close();
    } else {
      // If file didn't open, report this
      edm::LogError("SiPixelTemplateDBObjectUploader") << "Error opening File" << tempfile;
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
  edm::LogPrint("SiPixelTemplateDBObjectUploader") << "Phase-" << phase << " geometry is used \n" << std::endl;

  //Loop over the detector elements and put template IDs in place
  for (const auto& it : pDD->detUnits()) {
    if (it != nullptr) {
      // Here is the actual looping step over all DetIds:
      DetId detid = it->geographicalId();
      unsigned int layer = 0, ladder = 0, disk = 0, side = 0, blade = 0, panel = 0, module = 0;
      short thisID = 10000;
      unsigned int iter;

      // Now we sort them into the Barrel and Endcap:
      //Barrel Pixels first
      if ((phase == 1 && detid.subdetId() == static_cast<int>(PixelSubdetector::PixelBarrel)) ||
          (phase == 2 && tGeo->geomDetSubDetector(detid.subdetId()) == GeomDetEnumerators::P2PXB)) {
        //Get the layer, ladder, and module corresponding to this detID
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
            thisID = (short)theBarrelTemplateIds[iter];
        }

        if (thisID == 10000 || (!obj.putTemplateID(detid.rawId(), thisID)))
          edm::LogPrint("SiPixelTemplateDBObjectUploader")
              << " Could not fill barrel layer " << layer << ", module " << module << "\n";
        edm::LogPrint("SiPixelTemplateDBObjectUploader")
            << "This is a barrel element with: layer " << layer << ", ladder " << ladder << " and module " << module;
      }
      //Now endcaps
      else if ((phase == 1 && detid.subdetId() == static_cast<int>(PixelSubdetector::PixelEndcap)) ||
               (phase == 2 && tGeo->geomDetSubDetector(detid.subdetId()) == GeomDetEnumerators::P2PXEC)) {
        //Get the DetId's disk, blade, side, panel, and module
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
            thisID = (short)theEndcapTemplateIds[iter];
        }

        if (thisID == 10000 || (!obj.putTemplateID(detid.rawId(), thisID)))
          edm::LogPrint("SiPixelTemplateDBObjectUploader")
              << " Could not fill endcap det unit" << side << ", disk " << disk << ", blade " << blade << ", and panel "
              << panel << ".\n";
        edm::LogPrint("SiPixelTemplateDBObjectUploader") << "This is an endcap element with: side " << side << ", disk "
                                                         << disk << ", blade " << blade << ", and panel " << panel;
      } else {
        continue;
      }

      //Print out the assignment of this detID
      short mapnum;
      mapnum = obj.getTemplateID(detid.rawId());
      edm::LogPrint("SiPixelTemplateDBObjectUploader")
          << "The DetID: " << detid.rawId() << " is mapped to the template: " << mapnum << "\n";
    }
  }

  //--- Create a new IOV
  edm::Service<cond::service::PoolDBOutputService> poolDbService;
  if (!poolDbService.isAvailable())  // Die if not available
    throw cms::Exception("NotAvailable") << "PoolDBOutputService not available";
  if (poolDbService->isNewTagRequest("SiPixelTemplateDBObjectRcd"))
    poolDbService->writeOneIOV(obj, poolDbService->beginOfTime(), "SiPixelTemplateDBObjectRcd");
  else
    poolDbService->writeOneIOV(obj, poolDbService->currentTime(), "SiPixelTemplateDBObjectRcd");
}

void SiPixelTemplateDBObjectUploader::endJob() {}

DEFINE_FWK_MODULE(SiPixelTemplateDBObjectUploader);
