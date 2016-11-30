//====================================================================
//  lcgeo - LC detector models in DD4hep
//--------------------------------------------------------------------
//  DD4hep Geometry driver for SiWEcalBarrel
//  Ported from Mokka
//--------------------------------------------------------------------
//  S.Lu, DESY
//  D.Jeans, Tokyo <-- to 05: add possibility to remove preshower layer
//====================================================================

#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DetType.h"

#include "XML/Layering.h"
#include "TGeoTrd2.h"

#include "DDRec/Extensions/LayeringExtensionImpl.h"
#include "DDRec/Extensions/SubdetectorExtensionImpl.h"
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"

#include "DDSegmentation/MegatileLayerGridXY.h"
#include "DDSegmentation/WaferGridXY.h"

#include "SEcal05_Helpers.h"

#include <sstream>

#undef NDEBUG
#include <assert.h>

using namespace std;
using namespace DD4hep;
using namespace DD4hep::Geometry;

#define VERBOSE 1

// workaround for DD4hep v00-14 (and older)
#ifndef DD4HEP_VERSION_GE
#define DD4HEP_VERSION_GE(a,b) 0
#endif

/** SEcal05.cc
 *
 * new SEcal05 barrel driver: allows removal of preshower layer. DJeans UTokyo, sep/2016
 * based on SEcal04
 *
 *  @author: Shaojun Lu, DESY
 *  @version $Id: SEcal04_Barrel.cpp 1060 2016-09-05 07:48:41Z /C=JP/O=KEK/OU=CRC/CN=JEANS Daniel Thomelin Dietrich $
 *              Ported from Mokka SEcal04 Barrel part. Read the constants from XML
 *              instead of the DB. Then build the Barrel in the same way with DD4hep
 *              construct.
 *
 * @history: F.Gaede, CERN/DESY, Nov. 10, 2014
 *              added information for reconstruction: LayeringExtension and surfaces (experimental)
 *              removed DetElement for slices (not needed) increased multiplicity for layer DetElement
 *              along tower index
 *   F.Gaede: 03/2015:
 *            create the envelope volume with create_placed_envelope() using the xml
 */

/*


  y
  ^
  |     z is perpendicular to page (along beamline)
  |
  ----->  x

  original coord system was very mixed up.
  Daniel tried to rationalise it to make it more understandable (hopefully)

  absorber layers are wrapped in a number of layers of carbon fibre composite (CF)

  structure of slab (which is inserted into the structure) is defined in the compact description

  general structure (from +ve y)
  -------------------------------

  if preshower, front face (CF)               \\\\\\\\\\\\\\\\\\

  if no preshower, wrapped absorber           ==================

                                              __________________
  alveolus, containing the slab              |__________________|

  wrapped absorber                           ===================
                                              __________________
  alveolus, containing the slab              |__________________|

  .
  repeat as required
  .

  wrapped absorber                           ===================
                                             __________________
  alveolus, containing the slab             |__________________|

  back support plate (CF)                    \\\\\\\\\\\\\\\\\\\\
                                             ////////////////////

  */



static Ref_t create_detector(LCDD& lcdd, xml_h element, SensitiveDetector sens)  {

  cout << "-----------------------" << endl;
  cout << "creating SEcal05_Barrel" << endl;
  cout << "-----------------------" << endl;

  xml_det_t     x_det     = element;
  string        det_name  = x_det.nameStr();
  Layering      layering (element);

  Material      carbon_fibre = lcdd.material("CarbonFiber");

  int           det_id    = x_det.id();
  DetElement    sdet      (det_name,det_id);

  xml_comp_t    x_dim     = x_det.dimensions();
  int           nsides    = x_dim.numsides();

  // this code assumes octagon in several places
  //  would need work to make it general
  assert( nsides==8 );


  double        dphi      = (2*M_PI/nsides);
  double        hphi      = dphi/2;

  // --- create an envelope volume and position it into the world ---------------------

  Volume envelope = XML::createPlacedEnvelope( lcdd,  element , sdet ) ;

  XML::setDetectorTypeFlag( element, sdet ) ;

  if( lcdd.buildType() == BUILD_ENVELOPE ) return sdet ;

  //-----------------------------------------------------------------------------------

  sens.setType("calorimeter");

  DetElement    stave_det("module1stave1",det_id);

  //====================================================================
  //
  // Read all the constant from ILD_o1_v05.xml
  // Use them to build HcalBarrel
  //
  //====================================================================

  // some hardcoded values!
  const int N_FIBERS_W_STRUCTURE = 2; // number of CF layers around absorber layers in the structure
  const int N_FIBERS_ALVEOLUS    = 3; // number of CF layers used to make alveolus

  //  read other parameters from compact.xml file

  // overall size
  double Ecal_inner_radius                  = lcdd.constant<double>("Ecal_inner_radius");
  double Ecal_Barrel_halfZ                  = lcdd.constant<double>("Ecal_Barrel_halfZ");
  int    Ecal_barrel_z_modules              = lcdd.constant<int>("Ecal_barrel_z_modules");
  int    Ecal_barrel_number_of_towers       = lcdd.constant<int>("Ecal_barrel_number_of_towers");

  double Ecal_barrel_thickness              = lcdd.constant<double>("Ecal_barrel_thickness"); // what's assumed in the compact description

  // absorber layers
  int    Ecal_nlayers1                      = lcdd.constant<int>("Ecal_nlayers1");
  int    Ecal_nlayers2                      = lcdd.constant<int>("Ecal_nlayers2");
  int    Ecal_nlayers3                      = lcdd.constant<int>("Ecal_nlayers3");

  double Ecal_radiator_thickness1           = lcdd.constant<double>("Ecal_radiator_layers_set1_thickness");
  double Ecal_radiator_thickness2           = lcdd.constant<double>("Ecal_radiator_layers_set2_thickness");
  double Ecal_radiator_thickness3           = lcdd.constant<double>("Ecal_radiator_layers_set3_thickness");

  // CF support dimensions
  double Ecal_support_thickness             = lcdd.constant<double>("Ecal_support_thickness");
  double Ecal_front_face_thickness          = lcdd.constant<double>("Ecal_front_face_thickness");
  double Ecal_lateral_face_thickness        = lcdd.constant<double>("Ecal_lateral_face_thickness");
  double Ecal_Slab_H_fiber_thickness        = lcdd.constant<double>("Ecal_Slab_H_fiber_thickness");
  double Ecal_fiber_thickness               = lcdd.constant<double>("Ecal_fiber_thickness");
  double Ecal_Slab_shielding                = lcdd.constant<double>("Ecal_Slab_shielding");

  // first layer is preshower?
  bool   Ecal_Barrel_PreshowerLayer         = lcdd.constant<int>("Ecal_Barrel_Preshower") > 0;

  // internal dimensions of slab
  double Ecal_guard_ring_size               = lcdd.constant<double>("Ecal_guard_ring_size");
  int    Ecal_n_wafers_per_tower            = lcdd.constant<int>("Ecal_n_wafers_per_tower");

  std::string Ecal_layerConfig              = lcdd.constant<string>("Ecal_layer_pattern");

  int Ecal_end_of_slab_strategy             = lcdd.constant<int>("Ecal_end_of_slab_strategy");

  int Ecal_cells_across_megatile            = lcdd.constant <int> ("Ecal_cells_across_megatile" );
  int Ecal_strips_across_megatile           = lcdd.constant <int> ("Ecal_strips_across_megatile");
  int Ecal_strips_along_megatile            = lcdd.constant <int> ("Ecal_strips_along_megatile" );


  //---------------------------------

  // set up the helper, which will make a module for us

  SEcal05_Helpers helper;
  helper.setDet( & x_det );

  //   absorber layer structure
  helper.setPreshower( Ecal_Barrel_PreshowerLayer );

  cout << "Preshower ? " << Ecal_Barrel_PreshowerLayer << endl;

  helper.setAbsLayers( Ecal_nlayers1, Ecal_radiator_thickness1,
                       Ecal_nlayers2, Ecal_radiator_thickness2,
                       Ecal_nlayers3, Ecal_radiator_thickness3 );

  cout << "absorber layers " << 
    Ecal_nlayers1 << "*" << Ecal_radiator_thickness1/dd4hep::mm << " mm + " <<
    Ecal_nlayers2 << "*" << Ecal_radiator_thickness2/dd4hep::mm << " mm + " <<
    Ecal_nlayers3 << "*" << Ecal_radiator_thickness3/dd4hep::mm << " mm " << endl;

  helper.checkLayerConsistency();

  // structural CF thicknesses
  helper.setCFthickness( N_FIBERS_W_STRUCTURE*Ecal_fiber_thickness,
                         N_FIBERS_ALVEOLUS*Ecal_fiber_thickness,
                         Ecal_front_face_thickness,
                         Ecal_support_thickness);

  if ( !Ecal_Barrel_PreshowerLayer && Ecal_front_face_thickness>0 ) {
    cout << "WARNING, using front CF plate in a non-preshower setup: do you really want this?" << endl;
  }

  // check resulting thickness is consistent with what's in compact description
  float module_thickness = helper.getTotalThickness();
  if ( fabs( Ecal_barrel_thickness - module_thickness ) > 0.01 ) {
    cout << "ERROR : ECAL barrel thickness in comapct decription not consistent with what I calculate!" << endl;
    cout << "    calculated = " << module_thickness << "    compact description: " << Ecal_barrel_thickness << endl;
    assert( 0 ); // exit
  }

  cout << "module thickness = " << module_thickness << endl;

  // set up the sensitive layer segmentation

  Readout readout = sens.readout();
  DD4hep::Geometry::Segmentation seg = readout.segmentation();
  helper.setSegmentation( &seg );

  helper.setNCells( Ecal_cells_across_megatile, Ecal_strips_across_megatile, Ecal_strips_along_megatile);
  helper.setMagicMegatileStrategy ( Ecal_end_of_slab_strategy );

  // layer configuration
  int ntemp;
  stringstream stream(Ecal_layerConfig);
  std::vector < int > layerConfig;
  while ( stream >> ntemp ) {
    assert (ntemp>=0 );
    layerConfig.push_back( ntemp );
  }
  cout << "layer config: ";
  for (size_t i=0; i<layerConfig.size(); i++) cout << layerConfig[i] << " ";
  cout << endl;

  helper.setLayerConfig( layerConfig );


  // set the alveolar structure

  // get width of alveolus
  double Ecal_Barrel_module_dim_z = 2 * Ecal_Barrel_halfZ / Ecal_barrel_z_modules ;
  double alv_width = ( Ecal_Barrel_module_dim_z - 2.*Ecal_lateral_face_thickness ) / Ecal_barrel_number_of_towers;

  cout << "width of module, alveolus = " << Ecal_Barrel_module_dim_z << " " << alv_width << endl;

  std::vector < int > ntowers; ntowers.push_back(Ecal_barrel_number_of_towers);

  helper.setTowersUnits( ntowers,
			 alv_width,
			 Ecal_n_wafers_per_tower,
			 Ecal_lateral_face_thickness,
                         N_FIBERS_ALVEOLUS * Ecal_fiber_thickness + Ecal_Slab_H_fiber_thickness + Ecal_Slab_shielding,
                         Ecal_guard_ring_size );

  // shape of barrel module
  // to get alignment as in ECAL interface design doc fig10
  double module_thickness_noSupport = module_thickness - Ecal_support_thickness;

  double max_dim_x    = 2. * tan(M_PI/8.) * Ecal_inner_radius + module_thickness_noSupport/sin(M_PI/4.); // longest side
  double min_dim_x = max_dim_x - 2*module_thickness; // shorter one (assumes octagon)
  Trapezoid trd(max_dim_x / 2,
                min_dim_x / 2,
                Ecal_Barrel_module_dim_z / 2,
                Ecal_Barrel_module_dim_z / 2,
                module_thickness/2);

  Volume mod_vol(det_name+"_module", trd, carbon_fibre); // DJeans 5-sep-2016

  //========== fill data for reconstruction ============================
  DDRec::LayeredCalorimeterData* caloData = new DDRec::LayeredCalorimeterData ;

  helper.setModuleDimensions( 0, // int XYtype, // module shape in XY
                              1, // int XZtype, // module shape in XZ
                              max_dim_x // double dX_max, // maximum extent in X
                              );

  // traslation to get layers within the envelope
  helper.setTranslation ( DD4hep::Geometry::Position ( -max_dim_x/2. , -Ecal_Barrel_module_dim_z/2., -module_thickness/2. ) );

  // make the module

  helper.makeModule( mod_vol, stave_det,
		     *caloData,
		     lcdd, sens );

  for (size_t i=0; i<caloData->layers.size(); i++) {
    caloData->layers[i].distance += Ecal_inner_radius; // add IP->front face distance
  }

  cout << "cell sizes: " << endl;
  for (size_t i=0; i<caloData->layers.size(); i++) {
    cout << "sensitive layer " << i << " : x,y = " << caloData->layers[i].cellSize0 << " " << caloData->layers[i].cellSize1 << endl;
  }

  // ------------- create extension objects for reconstruction -----------------

  DDRec::LayeringExtensionImpl* layeringExtension = new DDRec::LayeringExtensionImpl ;
  DDRec::SubdetectorExtensionImpl* subDetExtension = new DDRec::SubdetectorExtensionImpl( sdet )  ;
  Position layerNormal(0,0,1); //fg: defines direction of thickness in Box for layer slices
  subDetExtension->setIsBarrel( true ) ;
  subDetExtension->setNSides( nsides ) ;
  subDetExtension->setRMin( Ecal_inner_radius ) ;
  subDetExtension->setRMax( ( Ecal_inner_radius + module_thickness ) / cos( M_PI/8. ) ) ; // this is to the external corner
  subDetExtension->setZMin( 0. ) ;
  subDetExtension->setZMax( Ecal_Barrel_halfZ ) ;

  caloData->layoutType = DDRec::LayeredCalorimeterData::BarrelLayout ;
  caloData->inner_symmetry = nsides  ;
  caloData->outer_symmetry = nsides  ;
  caloData->phi0 = 0 ; // hardcoded

  // extent of the calorimeter in the r-z-plane [ rmin, rmax, zmin, zmax ] in mm.
  caloData->extent[0] = Ecal_inner_radius ;
  caloData->extent[1] = ( Ecal_inner_radius + module_thickness );
  caloData->extent[2] = 0. ;
  caloData->extent[3] = Ecal_Barrel_halfZ ;
  //-------------------------------------------------------

  //====================================================================
  // Place ECAL Barrel stave module into the envelope volume
  //====================================================================

  double X = Ecal_inner_radius + module_thickness / 2.;

  // altered to get alignment as in ECAL interface design doc fig10
  //   end of slabs aligned to inner face of support plate in next stave (not the outer surface)
  // double Y = (module_thickness/2.) / sin(M_PI/4.);
  double Y = (module_thickness_noSupport/2.) / sin(M_PI/4.);

  // stave numbering from 1->8
  //   stave = 1 is in +ve x direction
  //   stave = 3 is in +ve y direction (ie at top of barrel)
  // module index from 1->5
  //   module = 1 is at -ve z

  for (int istave = 0; istave < nsides ; istave++) {
    int stave_id = istave+1;
    double phirot =  (istave) * dphi + hphi ;    // magic rotations to get the modules in the correct orientation/position
    double phirot2 =  (istave) * dphi + hphi - 2*dphi;
    for (int imodule = 0; imodule < Ecal_barrel_z_modules; imodule++) {
      int module_id = imodule+1;
      Transform3D tr( RotationZYX( 0 , phirot, M_PI/2.),  // magic rotation!
                      Translation3D(
                                    ( X*cos(phirot2)-Y*sin(phirot2) ) ,
                                    ( X*sin(phirot2)+Y*cos(phirot2) ) ,
                                    ( imodule+0.5 )*Ecal_Barrel_module_dim_z - Ecal_Barrel_halfZ )
                      );
      PlacedVolume pv = envelope.placeVolume(mod_vol,tr);
      pv.addPhysVolID("module",module_id);
      pv.addPhysVolID("stave",stave_id);

      DetElement sd = (imodule==0 && istave==0) ? stave_det : stave_det.clone(_toString(module_id,"module%d")+_toString(stave_id,"stave%d"));
      sd.setPlacement(pv);
      sdet.add(sd);
    }
  }

  // Set envelope volume attributes.
  envelope.setAttributes(lcdd,x_det.regionStr(),x_det.limitsStr(),x_det.visStr());

  sdet.addExtension< DDRec::LayeringExtension      >( layeringExtension ) ;
  sdet.addExtension< DDRec::SubdetectorExtension   >( subDetExtension ) ;
  sdet.addExtension< DDRec::LayeredCalorimeterData >( caloData ) ;

  //  cout << "finished SEcal05_Barrel" << endl;

  return sdet;
}

DECLARE_DETELEMENT(SEcal05_Barrel,create_detector)
