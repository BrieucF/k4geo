//====================================================================
//  lcgeo - LC detector models in DD4hep 
//--------------------------------------------------------------------
//  DD4hep Geometry driver for SiWEcalBarrel
//  Ported from Mokka
//--------------------------------------------------------------------
//  S.Lu, DESY
//  $Id: SEcal04Hybrid_Barrel.cpp 953 2016-04-20 08:28:15Z shaojun $
//====================================================================

#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DetType.h"
#include "XML/Layering.h"
#include "TGeoTrd2.h"

#include "DDRec/Extensions/LayeringExtensionImpl.h"
#include "DDRec/Extensions/SubdetectorExtensionImpl.h"
//#include "DDRec/Surface.h"
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"
#include "DDSegmentation/MegatileLayerGridXY.h"
//#include "DDSegmentation/WaferGridXY.h"

using namespace std;
using namespace DD4hep;
using namespace DD4hep::Geometry;

//#define VERBOSE 1
//#define VERBOSEcoterra 1

// workaround for DD4hep v00-14 (and older) 
#ifndef DD4HEP_VERSION_GE
#define DD4HEP_VERSION_GE(a,b) 0 
#endif

/** SEcal04.cc
 *
 *  @author: Shaojun Lu, DESY
 *  @version $Id: SEcal04Hybrid_Barrel.cpp 953 2016-04-20 08:28:15Z shaojun $
 *              Ported from Mokka SEcal04 Barrel part. Read the constants from XML
 *              instead of the DB. Then build the Barrel in the same way with DD4hep
 * 		construct.
 *
 * @history: F.Gaede, CERN/DESY, Nov. 10, 2014
 *              added information for reconstruction: LayeringExtension and surfaces (experimental)
 *              removed DetElement for slices (not needed) increased multiplicity for layer DetElement
 *              along tower index  
 *   F.Gaede: 03/2015: 
 *            create the envelope volume with create_placed_envelope() using the xml 
 */

static Ref_t create_detector(LCDD& lcdd, xml_h element, SensitiveDetector sens)  {
  static double tolerance = 0e0;

  xml_det_t     x_det     = element;
  string        det_name  = x_det.nameStr();
  Layering      layering (element);

  Material      air       = lcdd.air();
  Material      vacuum    = lcdd.vacuum();

  int           det_id    = x_det.id();
  xml_comp_t    x_staves  = x_det.staves();
  DetElement    sdet      (det_name,det_id);

  xml_comp_t    x_dim     = x_det.dimensions();
  int           nsides    = x_dim.numsides();
  double        dphi      = (2*M_PI/nsides);
  double        hphi      = dphi/2;


 // --- create an envelope volume and position it into the world ---------------------

  Volume envelope = XML::createPlacedEnvelope( lcdd,  element , sdet ) ;

  XML::setDetectorTypeFlag( element, sdet ) ;
 
  if( lcdd.buildType() == BUILD_ENVELOPE ) return sdet ;

  //-----------------------------------------------------------------------------------


  sens.setType("calorimeter");

  Material stave_material  = lcdd.material(x_staves.materialStr());

  DetElement    stave_det("module0stave0",det_id);

  Readout readout = sens.readout();
  Segmentation seg = readout.segmentation();
  
  // check if we have a WaferGridXY segmentation :
//  DD4hep::DDSegmentation::WaferGridXY* waferSeg = 
  DD4hep::DDSegmentation::MegatileLayerGridXY* EBUSeg = 
//    dynamic_cast< DD4hep::DDSegmentation::WaferGridXY*>( seg.segmentation() ) ;
	    dynamic_cast< DD4hep::DDSegmentation::MegatileLayerGridXY*>( seg.segmentation() ) ;

  std::vector<double> cellSizeVector = seg.segmentation()->cellDimensions(0); //Assume uniform cell sizes, provide dummy cellID
  double cell_sizeX      = cellSizeVector[0];
  double cell_sizeY      = cellSizeVector[1];
#if VERBOSEcoterra
	std::cout << "Start position of SEcal04Hybrid_Barrel: cell_sizeX = " << cell_sizeX << ", cell_sizeY = " <<  cell_sizeY << std::endl;
#endif
//====================================================================
//
// Read all the constant from ILD_o1_v05.xml
// Use them to build HcalBarrel
//
//====================================================================

  int N_FIBERS_W_STRUCTURE = 2; 
  int N_FIBERS_ALVOULUS = 3;

  //  read parametere from compact.xml file
  double Ecal_Alveolus_Air_Gap              = lcdd.constant<double>("Ecal_Alveolus_Air_Gap");
  double Ecal_Slab_shielding                = lcdd.constant<double>("Ecal_Slab_shielding");
  double Ecal_Slab_copper_thickness         = lcdd.constant<double>("Ecal_Slab_copper_thickness");
  double Ecal_Slab_PCB_thickness            = lcdd.constant<double>("Ecal_Slab_PCB_thickness");
  double Ecal_Slab_glue_gap                 = lcdd.constant<double>("Ecal_Slab_glue_gap");
  double Ecal_Slab_ground_thickness         = lcdd.constant<double>("Ecal_Slab_ground_thickness");
  double Ecal_fiber_thickness               = lcdd.constant<double>("Ecal_fiber_thickness");
  double Ecal_Si_thickness                  = lcdd.constant<double>("Ecal_Si_thickness");
  
  double Ecal_inner_radius                  = lcdd.constant<double>("TPC_outer_radius") +lcdd.constant<double>("Ecal_Tpc_gap");
  double Ecal_radiator_thickness1           = lcdd.constant<double>("Ecal_radiator_layers_set1_thickness");
  double Ecal_radiator_thickness2           = lcdd.constant<double>("Ecal_radiator_layers_set2_thickness");
  double Ecal_radiator_thickness3           = lcdd.constant<double>("Ecal_radiator_layers_set3_thickness");
  double Ecal_Barrel_halfZ                  = lcdd.constant<double>("Ecal_Barrel_halfZ");
  
  double Ecal_support_thickness             = lcdd.constant<double>("Ecal_support_thickness");
  double Ecal_front_face_thickness          = lcdd.constant<double>("Ecal_front_face_thickness");
  double Ecal_lateral_face_thickness        = lcdd.constant<double>("Ecal_lateral_face_thickness");
  double Ecal_Slab_H_fiber_thickness        = lcdd.constant<double>("Ecal_Slab_H_fiber_thickness");

  double Ecal_Slab_Sc_PCB_thickness         = lcdd.constant<double>("Ecal_Slab_Sc_PCB_thickness");
  double Ecal_Sc_thickness                  = lcdd.constant<double>("Ecal_Sc_thickness");
  double Ecal_Sc_reflector_thickness        = lcdd.constant<double>("Ecal_Sc_reflector_thickness");

  int    Ecal_nlayers1                      = lcdd.constant<int>("Ecal_nlayers1");
  int    Ecal_nlayers2                      = lcdd.constant<int>("Ecal_nlayers2");
  int    Ecal_nlayers3                      = lcdd.constant<int>("Ecal_nlayers3");
  int    Ecal_barrel_number_of_towers       = lcdd.constant<int>("Ecal_barrel_number_of_towers");
  
  //double      Ecal_cells_size                  = lcdd.constant<double>("Ecal_cells_size");
  double Ecal_guard_ring_size               = lcdd.constant<double>("Ecal_guard_ring_size");

	// 2016/0505 for strip
	int 	 Ecal_Sc_N_strips_across_module     = lcdd.constant<int>("Ecal_Sc_N_strips_across_module");
	int 	 Ecal_Sc_number_of_virtual_cells	  = lcdd.constant<int>("Ecal_Sc_number_of_virtual_cells");
	// 20160514 for ScECAL
	int 	 Ecal_n_wafers_per_tower; //					  = lcdd.constant<int>("Ecal_n_wafers_per_tower");
#ifdef VERBOSEcoterra
  std::cout << " Ecal_Sc_N_strips_across_module = " << Ecal_Sc_N_strips_across_module  << " for ScECAL" << std::endl;
 // std::cout << " Ecal_n_wafers_per_tower = " << Ecal_n_wafers_per_tower  << " ;1 for  ScECAL" << std::endl;
#endif 
 
//====================================================================
//
// general calculated parameters
//
//====================================================================
  
  double Ecal_total_SiSlab_thickness = 
    Ecal_Slab_shielding + 
    Ecal_Slab_copper_thickness + 
    Ecal_Slab_PCB_thickness +
    Ecal_Slab_glue_gap + 
    Ecal_Si_thickness + 
    Ecal_Slab_ground_thickness +
    Ecal_Alveolus_Air_Gap / 2;
#ifdef VERBOSE
  std::cout << " Ecal_total_SiSlab_thickness = " << Ecal_total_SiSlab_thickness  << std::endl;
#endif
  
  

  double Ecal_total_ScSlab_thickness = 
    Ecal_Slab_shielding + 
    Ecal_Slab_copper_thickness + 
    Ecal_Slab_Sc_PCB_thickness +
    Ecal_Sc_thickness + 
    Ecal_Sc_reflector_thickness * 2 +
    Ecal_Alveolus_Air_Gap / 2;
#ifdef VERBOSE
  std::cout << " Ecal_total_ScSlab_thickness = " << Ecal_total_ScSlab_thickness  << std::endl;
#endif
  

	int Number_of_Si_Layers_in_Barrel = 0;
	int Number_of_Sc_Layers_in_Barrel = 0;


#ifdef VERBOSE
  std::cout << " Ecal total number of Silicon layers = " << Number_of_Si_Layers_in_Barrel  << std::endl;
  std::cout << " Ecal total number of Scintillator layers = " << Number_of_Sc_Layers_in_Barrel  << std::endl;
#endif
  
  // In this release the number of modules is fixed to 5
  double Ecal_Barrel_module_dim_z = 2 * Ecal_Barrel_halfZ / 5. ;
#ifdef VERBOSE
  std::cout << "Ecal_Barrel_module_dim_z  = " << Ecal_Barrel_module_dim_z  << std::endl;
#endif

  // The alveolus size takes in account the module Z size
  // but also 4 fiber layers for the alveoulus wall, the all
  // divided by the number of towers

  // "alveolus_dim_z" is starting size of all.
  double alveolus_dim_z =  
    (Ecal_Barrel_module_dim_z - 2. * Ecal_lateral_face_thickness) /
    Ecal_barrel_number_of_towers - 
    2 * N_FIBERS_ALVOULUS  * Ecal_fiber_thickness  - 
    2 * Ecal_Slab_H_fiber_thickness -
    2 * Ecal_Slab_shielding;


#ifdef VERBOSE
  std::cout << "alveolus_dim_z = " <<  alveolus_dim_z << std::endl;
#endif

  int n_total_layers = Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3;
  //TODO xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxNumber_of_Si_Layers_in_Barrel = n_total_layers+1;
  Number_of_Sc_Layers_in_Barrel = n_total_layers+1;

  double module_thickness = 
    Ecal_nlayers1 * Ecal_radiator_thickness1 +
    Ecal_nlayers2 * Ecal_radiator_thickness2 +
    Ecal_nlayers3 * Ecal_radiator_thickness3 +
    
		int(n_total_layers/2) * // fiber around W struct layers
    (N_FIBERS_W_STRUCTURE * 2 *  Ecal_fiber_thickness) +
    
    Number_of_Si_Layers_in_Barrel * // Silicon slabs plus fiber around and inside
    (Ecal_total_SiSlab_thickness +
     (N_FIBERS_ALVOULUS + 1 ) * Ecal_fiber_thickness) +
    
    Number_of_Sc_Layers_in_Barrel * // Scintillator slabs plus fiber around and inside
    (Ecal_total_ScSlab_thickness +
     (N_FIBERS_ALVOULUS + 1 ) * Ecal_fiber_thickness) +
    
    Ecal_support_thickness + Ecal_front_face_thickness;
  
#ifdef VERBOSE
  std::cout << "For information : module_thickness = " << module_thickness  << std::endl;
#endif
  
  // module barrel key parameters
  double  bottom_dim_x = 2. * tan(M_PI/8.) * Ecal_inner_radius +
    module_thickness/sin(M_PI/4.);
  
  double top_dim_x = bottom_dim_x - 2 * module_thickness;

  //------------------------------------------------------------------------------------

  DDRec::LayeredCalorimeterData::Layer caloLayer ;
  caloLayer.cellSize0 = cell_sizeX;
  caloLayer.cellSize1 = cell_sizeY;

  //== For Wafer ===  
  double cell_dim_x = caloLayer.cellSize0;
//  double total_Si_dim_z = alveolus_dim_z;
#ifdef VERBOSEcoterra
  std::cout << "cell_dim_x = " << cell_dim_x 	<< ", caloLayer.cellSize0 = " << caloLayer.cellSize0
  																						<< ", caloLayer.cellSize1 = " << caloLayer.cellSize1 << std::endl;
#endif

	Ecal_n_wafers_per_tower = EBUSeg->NMegaY();
  double util_SI_wafer_dim_z = 
    // one EBU in an alveolus_dim_z::: total_Si_dim_z/2 -  2 * Ecal_guard_ring_size;
//    total_Si_dim_z -  2 * Ecal_guard_ring_size;
    alveolus_dim_z -  2 * Ecal_guard_ring_size;

//  double cell_dim_z =  util_SI_wafer_dim_z/ 
//    floor(util_SI_wafer_dim_z/
//	  cell_dim_x);
//	 for ScECAL cell length z is temporal.
	int nStripX = EBUSeg->NStripsX();
	int nStripY = EBUSeg->NStripsY();
  int Num_of_minCellUnitInEBU   = std::max( nStripX, nStripY );
  int Num_of_minCellUnitInStrip = Num_of_minCellUnitInEBU / std::min( nStripX, nStripY );
	double residualT_L =  (double)Num_of_minCellUnitInEBU / std::min( nStripX, nStripY) - Num_of_minCellUnitInStrip;
	if ( residualT_L > 0 ) {

		std::cout << "WARNING, strip length/strip width is not a integer" << std::endl;

	}

#ifdef VERBOSEcoterra
	std::cout << "Num_of_minCellUnitInEBU = " << Num_of_minCellUnitInEBU << ", nStripX(Y) = " << nStripX << ", " << nStripY << std::endl;
#endif
  double cell_dim_z =  util_SI_wafer_dim_z/ Num_of_minCellUnitInEBU;
						//g519				(Ecal_Sc_N_strips_across_module * Ecal_Sc_number_of_virtual_cells) ; 

//// the following might can be "N_cells_in_Z = Ecal_Sc_N_strips_across_module"
//  int N_cells_in_Z = int(util_SI_wafer_dim_z/cell_dim_z);
//  int N_cells_in_X = N_cells_in_Z;
  
  cell_dim_x = cell_dim_z; // Those sizes are square cell dimension.
#ifdef VERBOSEcoterra
	std::cout << " util_SI_wafer_dim_z = " << util_SI_wafer_dim_z << std::endl;
	std::cout << " cell_dim_z = " << cell_dim_z << std::endl;
	std::cout << " cell_dim_x = " << cell_dim_x << std::endl;
//	std::cout << " N_cells_in_Z = " << N_cells_in_Z << std::endl;
#endif
  
#ifdef VERBOSE
  std::cout << " bottom_dim_x = " << bottom_dim_x  << std::endl;
  std::cout << " top_dim_x = " << top_dim_x << std::endl;
  std::cout << " Ecal total number of Silicon layers = " << Number_of_Si_Layers_in_Barrel  << std::endl;
  std::cout << " Ecal total number of Scintillator layers = " << Number_of_Sc_Layers_in_Barrel  << std::endl;
#endif
  


// ========= Create Ecal Barrel stave   ====================================
//  It will be the volume for palcing the Ecal Barrel alveolus(i.e. Layers).
//  And the structure W plate.
//  Itself will be placed into the world volume.
// ==========================================================================

  // The TOP_X and BOTTOM_X is different in Mokka and DD4hep
  Trapezoid trd(top_dim_x / 2,
		bottom_dim_x / 2, 
		Ecal_Barrel_module_dim_z / 2,
		Ecal_Barrel_module_dim_z / 2,
		module_thickness/2);

  Volume mod_vol(det_name+"_module",trd,air);

  // We count the layers starting from IP and from 1,
  // so odd layers should be inside slabs and
  // even ones on the structure.
  // The structure W layers are here big plans, as the 
  // gap between each W plate is too small to create problems 
  // The even W layers are part of H structure placed inside
  // the alveolus.

  // ############################
  //  Dimension of radiator wLog
  //  slice provide the thickness
  // ############################

  // y_floor is also thickness of side wall.
  double y_floor = 
    Ecal_front_face_thickness +
    N_FIBERS_ALVOULUS * Ecal_fiber_thickness;

  // ############################
  //  Dimension of alveolus
  //  slice provide the thickness
  // ############################
  
    // =====  build Si Slab and put into the Layer volume =====
    // =====  place the layer into the module 5 time for one full layer into the trd module ====
    // =====  build and place barrel structure into trd module ====
    // Parameters for computing the layer X dimension:
	double stave_z  =(Ecal_Barrel_module_dim_z - 2. * Ecal_lateral_face_thickness) / Ecal_barrel_number_of_towers/2.;
	double l_dim_x  = bottom_dim_x/2.;                            // Starting X dimension for the layer.
	double l_pos_z  = module_thickness/2;

	l_dim_x -= y_floor; //dimension of the layer.
	l_pos_z -= y_floor;

#ifdef VERBOSEcoterra
	std::cout << "y_floor = " << y_floor <<", l_dim_x = " << l_dim_x << ", l_pos_z = " << l_pos_z << std::endl;
#endif

	// ------------- create extension objects for reconstruction -----------------
	DDRec::LayeringExtensionImpl* layeringExtension = new DDRec::LayeringExtensionImpl ;
	DDRec::SubdetectorExtensionImpl* subDetExtension = new DDRec::SubdetectorExtensionImpl( sdet )  ;
	Position layerNormal(0,0,1); //fg: defines direction of thickness in Box for layer slices
    
	subDetExtension->setIsBarrel(true) ;
	subDetExtension->setNSides( 8 ) ;
	//    subDetExtension->setPhi0( 0 ) ;
	subDetExtension->setRMin( Ecal_inner_radius ) ;
	subDetExtension->setRMax( ( Ecal_inner_radius + module_thickness ) / cos( M_PI/8. ) ) ;
	subDetExtension->setZMin( 0. ) ;
	subDetExtension->setZMax( Ecal_Barrel_halfZ ) ;
    
	//========== fill data for reconstruction ============================
	DDRec::LayeredCalorimeterData* caloData = new DDRec::LayeredCalorimeterData ;
	caloData->layoutType = DDRec::LayeredCalorimeterData::BarrelLayout ;
	caloData->inner_symmetry = nsides  ;
	//added by Thorben Quast
	caloData->outer_symmetry = nsides  ;
	caloData->phi0 = 0 ; // hardcoded 
    
	/// extent of the calorimeter in the r-z-plane [ rmin, rmax, zmin, zmax ] in mm.
	caloData->extent[0] = Ecal_inner_radius ;
	//line fixed by Thorben Quast since actual conversion is made during the drawing
	caloData->extent[1] = ( Ecal_inner_radius + module_thickness );
	//caloData->extent[1] = ( Ecal_inner_radius + module_thickness ) / cos( M_PI/8. ) ;
	caloData->extent[2] = 0. ;
	caloData->extent[3] = Ecal_Barrel_halfZ ;

    // // base vectors for surfaces:
    // DDSurfaces::Vector3D u(1,0,0) ;
    // DDSurfaces::Vector3D v(0,1,0) ;
    // DDSurfaces::Vector3D n(0,0,1) ;

	//-------------------- start loop over ECAL layers ----------------------
	// Loop over the sets of layer elements in the detector.

	double nRadiationLengths   = 0. ;
	double nInteractionLengths = 0. ;
	double thickness_sum       = 0. ;

	nRadiationLengths   = Ecal_radiator_thickness1/(stave_material.radLength())
      + y_floor/air.radLength();
	nInteractionLengths = Ecal_radiator_thickness1/(stave_material.intLength())
      + y_floor/air.intLength();
	thickness_sum       = Ecal_radiator_thickness1 + y_floor;

	int l_num = 1;
// no use g630	int sensLayer = 0;
	bool isFirstSens = true;
	int myLayerNum = 0 ;

	for(xml_coll_t li(x_det,_U(layer)); li; ++li)  { // loop for layer types
		xml_comp_t x_layer = li;
		int repeat = x_layer.repeat();
		// Loop over number of repeats for this layer.
		for (int j=0; j<repeat; j++)    {								// loop for this type layers
			string l_name = _toString(l_num,"layer%d");
			double l_thickness = layering.layer(l_num-1)->thickness();  // Layer's thickness.
			double xcut = (l_thickness);                     // X dimension for this layer.
			l_dim_x -= xcut; // because 45d

			Box        l_box(l_dim_x-tolerance,stave_z-tolerance,l_thickness/2.0-tolerance);
			Volume     l_vol(det_name+"_"+l_name,l_box,air);

			l_vol.setVisAttributes(lcdd.visAttributes(x_layer.visStr()));

			//fg: need vector of DetElements for towers ! 
			//    DetElement layer(stave_det, l_name, det_id);
			std::vector< DetElement > layers( Ecal_barrel_number_of_towers )  ;
	
			// place layer 5 times in module. at same layer position (towers !)
			double l_pos_y = Ecal_Barrel_module_dim_z / 2. 
				  - ( Ecal_lateral_face_thickness +
							Ecal_fiber_thickness * N_FIBERS_ALVOULUS +
	      			Ecal_Slab_shielding + 
	      			Ecal_Slab_H_fiber_thickness +
	      			alveolus_dim_z /2.);						  
			for (int i=0; i<Ecal_barrel_number_of_towers; i++){ // need four clone <-- coterra thinks more than four ...
				layers[i] = DetElement( stave_det, l_name+_toString(i,"tower%02d") , det_id ) ;
				Position   l_pos(0,l_pos_y,l_pos_z-l_thickness/2.);      // Position of the layer.
				PlacedVolume layer_phv = mod_vol.placeVolume(l_vol,l_pos);
	  		// layer_phv.addPhysVolID("layer", l_num);
				layer_phv.addPhysVolID("tower", i);

				layers[i].setPlacement(layer_phv);
				l_pos_y -= (alveolus_dim_z + 
		      2. * Ecal_fiber_thickness * N_FIBERS_ALVOULUS +
		      2. * Ecal_Slab_H_fiber_thickness +
		      2. * Ecal_Slab_shielding);
			}  /// the end of loop of the four clone

			// Loop over the sublayers or slices for this layer.
			int s_num = 1;
			double s_pos_z = l_thickness / 2.;

			//--------------------------------------------------------------------------------
			// BuildBarrelAlveolus: BuildSiliconSlab:
			//--------------------------------------------------------------------------------
			double radiator_dim_y = Ecal_radiator_thickness1; //to be updated with slice radiator thickness 

			for(xml_coll_t si(x_layer,_U(slice)); si; ++si)  {
				xml_comp_t x_slice = si;
				string     s_name  =  _toString(s_num,"slice%d");
				double     s_thick = x_slice.thickness();
				Material slice_material  = lcdd.material(x_slice.materialStr());
#ifdef VERBOSE
	  		std::cout<<"Ecal_barrel_number_of_towers: "<< Ecal_barrel_number_of_towers <<std::endl;
#endif
				double slab_dim_x = l_dim_x-tolerance;
				double slab_dim_y = s_thick/2.;
				double slab_dim_z = stave_z-tolerance;

				Box        s_box(slab_dim_x,slab_dim_z,slab_dim_y);
				Volume     s_vol(det_name+"_"+l_name+"_"+s_name,s_box,slice_material);
	  //fg: not needed          DetElement slice(layer,s_name,det_id);

				s_vol.setVisAttributes(lcdd.visAttributes(x_slice.visStr()));
#ifdef VERBOSE
	  		std::cout<<"x_slice.materialStr(): "<< x_slice.materialStr() <<std::endl;
#endif
				if (x_slice.materialStr().compare(x_staves.materialStr()) == 0){
	    		radiator_dim_y = s_thick;
	  		// W StructureLayer has the same thickness as W radiator layer in the Alveolus layer
	    
#if DD4HEP_VERSION_GE( 0, 15 )
	    		caloLayer.outer_nRadiationLengths   = nRadiationLengths;
	    		caloLayer.outer_nInteractionLengths = nInteractionLengths;
	    		caloLayer.outer_thickness           = thickness_sum;

	    		//Only fill the layers information into DDRec after first layer as Mokka Gear.
	    		caloLayer.thickness = caloLayer.inner_thickness + caloLayer.outer_thickness ;
	    		if (!isFirstSens){ caloData->layers.push_back( caloLayer ) ;
#ifdef VERBOSE
	    			std::cout<<" caloLayer.distance: "<< caloLayer.distance <<std::endl; //cm THis might be dist. from IP.
	    			std::cout<<" caloLayer.inner_nRadiationLengths: "<< caloLayer.inner_nRadiationLengths <<std::endl;
			    	std::cout<<" caloLayer.inner_nInteractionLengths: "<< caloLayer.inner_nInteractionLengths <<std::endl;
	  		  	std::cout<<" caloLayer.inner_thickness: "<< caloLayer.inner_thickness <<std::endl;
			    	std::cout<<" caloLayer.sensitive_thickness: "<< caloLayer.sensitive_thickness <<std::endl;
			    	std::cout<<" caloLayer.outer_nRadiationLengths: "<< caloLayer.outer_nRadiationLengths <<std::endl;
	  		  	std::cout<<" caloLayer.outer_nInteractionLengths: "<< caloLayer.outer_nInteractionLengths <<std::endl;
			    	std::cout<<" caloLayer.outer_thickness: "<< caloLayer.outer_thickness <<std::endl;
						std::cout<<" EcalBarrel[1]==>caloLayer.inner_thickness + caloLayer.outer_thickness: "
		    	 		<< caloLayer.inner_thickness + caloLayer.outer_thickness <<std::endl;
#endif
#endif
	    		} // end of writing layer properties
	    		// Init for inner
			    nRadiationLengths   = 0. ;
			    nInteractionLengths = 0. ;
		  	  thickness_sum       = 0. ;	    
			    isFirstSens         = false;

	  		} // 
				nRadiationLengths   += s_thick/(2.*slice_material.radLength());
				nInteractionLengths += s_thick/(2.*slice_material.intLength());
				thickness_sum       += s_thick/2.;

///////////////////////////////////////////////////////
/////////// Start sensor layers are implemented. //////
				if ( x_slice.isSensitive() ) {
					// g603 not use sensLayer ++;
	    		//s_vol.setSensitiveDetector(sens);
			    // Normal squared wafers
	    		double wafer_dim_x = alveolus_dim_z / Ecal_n_wafers_per_tower;//TODO move to more global place.
// g628.1513
					wafer_dim_x = wafer_dim_x - 2 * Ecal_guard_ring_size;
//	      		N_cells_in_X * cell_dim_x;
	    		double wafer_dim_z = wafer_dim_x; // alveolus_dim_z / Ecal_n_wafers_per_tower;
//	      		N_cells_in_Z * cell_dim_z;
#if VERBOSEcoterra
					std::cout << "Normal WaferSiSolid: wafer_dim_x,y,z = " <<  wafer_dim_x << ", " << wafer_dim_z << ", " << slab_dim_y << std::endl;
#endif 
					double  wafer_dim_x_half = wafer_dim_x / 2.;
					double  wafer_dim_z_half = wafer_dim_z / 2.;
	    		Box WaferSiSolid( wafer_dim_x_half, wafer_dim_z_half, slab_dim_y);
	    		//Volume WaferSiLog(det_name+"_"+l_name+"_"+s_name+"Wafer",WaferSiSolid,slice_material);
	    		//WaferSiLog.setSensitiveDetector(sens);

	    		double real_wafer_size_x =
	      	wafer_dim_x + 2 * Ecal_guard_ring_size;


					double stripLength = cell_dim_x * Num_of_minCellUnitInStrip; // cell_dim_x is square cell's dimension. 
	    		int n_wafers_x =
							// slab_dim_x is half size of slab dimension in x. g0509 coterra
	      			/////////////////int(floor(slab_dim_x*2 / real_wafer_size_x)); 
	      			//Regulation EBU is necessary longer than one strip in x.
	      			int(floor( (slab_dim_x*2 - stripLength) / real_wafer_size_x));       
	    		double wafer_pos_x = // start position x. g0509 coterra
	      			-slab_dim_x - real_wafer_size_x /2;
	      //			Ecal_guard_ring_size +
	      //			wafer_dim_x /2 ;
	    		int n_wafer_x;
	    		int wafer_num = 0;
#if VERBOSEcoterra
					std::cout << " layer = " << l_num << ", wafer_dim_x = " << wafer_dim_x << ", wafer_dim_z = " <<  wafer_dim_z
										<< ", real_wafer_size_x = " << real_wafer_size_x << ", n_wafers_x = " << n_wafers_x
										<< ", n_wafer_x = " << n_wafer_x << ", wafer_pos_x = " << wafer_pos_x
										<< std::endl;								
#endif

			    for (n_wafer_x = 1;
							 n_wafer_x < n_wafers_x + 1; n_wafer_x++) { // start Wafer loop
						double wafer_pos_z =
								-alveolus_dim_z/2.0 + 
								Ecal_guard_ring_size +
								wafer_dim_z /2;

						wafer_pos_x += real_wafer_size_x; // wafer_dim_x + 2 * Ecal_guard_ring_size;
						//////////////////////////////////////////
						// One EBU occupies one alveolus width  
						for (int n_wafer_z = 1; n_wafer_z < Ecal_n_wafers_per_tower + 1; n_wafer_z++) { // start 2 rows in a tower
						//////////////////////////////////////////
							wafer_num++;
		    			string Wafer_name  =  _toString(wafer_num,"wafer%d");
							Volume WaferSiLog(det_name+"_"+l_name+"_"+s_name+"_"+Wafer_name,WaferSiSolid,slice_material);
							WaferSiLog.setSensitiveDetector(sens);
		    			//WaferSiLog.setVisAttributes(lcdd.visAttributes(x_slice.visStr()));
							PlacedVolume wafer_phv = s_vol.placeVolume(WaferSiLog,Position(wafer_pos_x, wafer_pos_z, 0));
							wafer_phv.addPhysVolID("wafer", wafer_num);

		    			// Normal squared wafers, this waferOffsetX is 0.0 
		    			//////////////////////////////waferSeg->setWaferOffsetX(myLayerNum, wafer_num, 0.0);
							/// Here we set the  MegatileLayerGridXY parameters.	
//							EBUSeg->setSlabOffsetX( l_num, wafer_num, slab_dim_x ); //////////////// coterra 
//							EBUSeg->setSlabOffsetY( l_num, wafer_num, slab_dim_z ); //////////////// coterra 
							EBUSeg->setWaferOffsetX( myLayerNum, wafer_num, wafer_dim_x_half ); //////////////// coterra 
							EBUSeg->setWaferOffsetY( myLayerNum, wafer_num, wafer_dim_z_half ); //////////////// coterra 
//							EBUSeg->setDeadWidth( Ecal_guard_ring_size );
//							EBUSeg->setWaferIndexX( l_num, n_wafer_x );
//							EBUSeg->setWaferIndexY( l_num, n_wafer_z );
//							EBUSeg->setNMegaY( Ecal_n_wafers_per_tower );//TODO move to more global place.
							EBUSeg->setTotalSizeX( myLayerNum, slab_dim_x * 2.0 );
							EBUSeg->setTotalSizeY( alveolus_dim_z );
//							EBUSeg->setFieldNameX( 
							wafer_pos_z += wafer_dim_z + 2 * Ecal_guard_ring_size;
						} // the end of rows in a tower
						//////////////////////////////////////////
#if VERBOSEcoterra
					std::cout << " layer = " << myLayerNum << ", wafer_dim_x = " << wafer_dim_x << ", wafer_dim_z = " <<  wafer_dim_z
										<< ", real_wafer_size_x = " << real_wafer_size_x << ", n_wafers_x = " << n_wafers_x
										<< ", n_wafer_x = " << n_wafer_x << ", wafer_pos_z = " << wafer_pos_z
										<< std::endl;
					std::cout //<< " offsetY = " << EBUSeg->offsetY() << ", offsetX = " << EBUSeg->offsetX()
//										<< ", deadWidth = " << EBUSeg->deadWidth() 
//										<< ", totalSizeX = " << EBUSeg->totalSizeX( l_num ) << ", totalSizeY = " << EBUSeg->totalSizeY()
										<< std::endl;
#endif

					} // the end of wafer loop

			    // Magic wafers to complete the slab...
	 		    // (wafers with variable number of cells just
			    // to complete the slab. in reality we think that
	  		  // we'll have just a few models of special wafers
	  		  // for that.
//	  		  int isLongStripInX = EBUSeg->isLongStripInX() // this returns 0 if strips do not along x 
																												//and returns how many times of square dimention the strips have,
																												//if the strips along x.
	    		double resting_dim_x = slab_dim_x*2 - (wafer_dim_x + 2 * Ecal_guard_ring_size) * 
	      				n_wafers_x;

	    		//f(resting_dim_x > (cell_dim_x + 2 * Ecal_guard_ring_size)) {
	    		// We make an out most EBU (regulation EBU) from stripLength to ( real_wafer_size_x + stripLegth ).

					EBUSeg->setIsRegulatingEBU( myLayerNum, wafer_num, false );
	    		if(resting_dim_x > (cell_dim_x + 2 * Ecal_guard_ring_size + stripLength )) {
						int N_cells_x_remaining = int(floor((resting_dim_x - 2 * Ecal_guard_ring_size)
			    														/cell_dim_x));
		
						//Here we need to make a Magic wafer!
						double regulatingWafer_dim_x = N_cells_x_remaining * cell_dim_x;
						double regulatingWafer_dim_x_half = regulatingWafer_dim_x / 2.0;
//      			EBUSeg->setWaferOffsetX( wafer_dim_x/2 ); //////////////// coterra 

#if VERBOSEcoterra
						std::cout << " regulatingWafer_dim_x = " << regulatingWafer_dim_x << std::endl;
						std::cout << "l_num: "<<l_num 
							<< ", N_cells_x_remaining = " << N_cells_x_remaining
							<< ", regulatingWafer_dim_x_half = " << regulatingWafer_dim_x_half
							<< std::endl;
#endif
						Box MagicWaferSiSolid( regulatingWafer_dim_x/2,wafer_dim_z/2,slab_dim_y);
						//Volume MagicWaferSiLog(det_name+"_"+l_name+"_"+s_name+"MagicWafer",MagicWaferSiSolid,slice_material);

						// Magic wafers, this waferOffsetX has to be taken care, 0.0 or half cell size in X.
						double thisWaferOffsetX = 0.0;
///////						if ( N_cells_x_remaining%2 ) thisWaferOffsetX = cell_dim_x/2.0;
//						wafer_pos_x = -slab_dim_x + n_wafers_x * real_wafer_size_x +
//													( regulatingWafer_dim_x + 2 * Ecal_guard_ring_size )/2;
						wafer_pos_x += real_wafer_size_x/2 +
													( regulatingWafer_dim_x + 2 * Ecal_guard_ring_size )/2;
  
//						double regulatingReal_wafer_size_x = regulatingWafer_dim_x + 2 * Ecal_guard_ring_size;
						// g628.1910 double wafer_pos_z = -alveolus_dim_z/2.0 + Ecal_guard_ring_size + regulatingWafer_dim_x /2;
						double wafer_pos_z = -alveolus_dim_z/2.0 + Ecal_guard_ring_size + wafer_dim_z /2;

						//int MagicWafer_num = 0;
						for (int n_wafer_z = 1; n_wafer_z < Ecal_n_wafers_per_tower + 1; n_wafer_z++) {
		    			wafer_num++;
		    			string MagicWafer_name  =  _toString(wafer_num,"MagicWafer%d");
		    			Volume MagicWaferSiLog(det_name+"_"+l_name+"_"+s_name+"_"+MagicWafer_name,MagicWaferSiSolid,slice_material);
		    			MagicWaferSiLog.setSensitiveDetector(sens);
							//MagicWaferSiLog.setVisAttributes(lcdd.visAttributes(x_slice.visStr()));
		    			PlacedVolume wafer_phv = s_vol.placeVolume(MagicWaferSiLog,Position(wafer_pos_x,
							       wafer_pos_z,
							       0));
		    			wafer_phv.addPhysVolID("wafer", wafer_num);
//		    			wafer_phv.addPhysVolID("waferY", wafer_num);

							//EBUSeg->setSlabOffsetX( l_num, slab_dim_x ); //////////////// coterra 
							//EBUSeg->setSlabOffsetY( l_num, slab_dim_z ); //////////////// coterra
							EBUSeg->setIsRegulatingEBU( myLayerNum, wafer_num, true );
							EBUSeg->setWaferOffsetX( myLayerNum, wafer_num, regulatingWafer_dim_x_half ); //////////////// coterra 
							EBUSeg->setWaferOffsetY( myLayerNum, wafer_num, wafer_dim_z_half ); //////////////// coterra 
//							EBUSeg->setDeadWidth( Ecal_guard_ring_size );
//							EBUSeg->setWaferIndexX( l_num, n_wafer_x );
//							EBUSeg->setWaferIndexY( l_num, n_wafer_z );
							EBUSeg->setTotalSizeX( myLayerNum, slab_dim_x * 2.0 );
							EBUSeg->setTotalSizeY( alveolus_dim_z );

		    			// Magic wafers, set the waferOffsetX for this layer this wafer.
		    			///////////////////////////////////waferSeg->setWaferOffsetX(myLayerNum, wafer_num, thisWaferOffsetX);

		    			wafer_pos_z += wafer_dim_z + 2 * Ecal_guard_ring_size;
						}
					}/// The end of MagicWafer
	    

#if DD4HEP_VERSION_GE( 0, 15 )
			    //Store "inner" quantities
	  		  caloLayer.inner_nRadiationLengths   = nRadiationLengths ;
	    		caloLayer.inner_nInteractionLengths = nInteractionLengths ;
	    		caloLayer.inner_thickness           = thickness_sum ;
	    		//Store sensitive slice thickness
	    		caloLayer.sensitive_thickness       = s_thick ;
#ifdef VERBOSE	    
	    		std::cout<<" l_num: "<<l_num <<std::endl;
	    		std::cout<<" s_num: "<<s_num <<std::endl;
	    		std::cout<<" Ecal_inner_radius: "<< Ecal_inner_radius <<std::endl;
	    		std::cout<<" module_thickness: "<< module_thickness <<std::endl;
	    		std::cout<<" l_pos_z: "<< l_pos_z <<std::endl;
	    		std::cout<<" l_thickness: "<< l_thickness <<std::endl;
	    		std::cout<<" s_pos_z: "<< s_pos_z <<std::endl;
	    		std::cout<<" s_thick: "<< s_thick <<std::endl;
	    		std::cout<<" radiator_dim_y: "<< radiator_dim_y <<std::endl;
#endif	    
	    		//-----------------------------------------------------------------------------------------
	    		caloLayer.distance  = Ecal_inner_radius + module_thickness/2.0 - l_pos_z + l_thickness/2. + (s_pos_z+s_thick/2.) 
	      												- caloLayer.inner_thickness;
					caloLayer.absorberThickness = radiator_dim_y ;

	    		//-----------------------------------------------------------------------------------------
#endif
	    		// Init for outer
	    		nRadiationLengths   = 0. ;
	    		nInteractionLengths = 0. ;
	    		thickness_sum       = 0. ;

				} // the end of "if sensivive slice" roop

	  		nRadiationLengths   += s_thick/(2.*slice_material.radLength());
	  		nInteractionLengths += s_thick/(2.*slice_material.intLength());
	  		thickness_sum       += s_thick/2;

        // Slice placement.
        PlacedVolume slice_phv = l_vol.placeVolume(s_vol,Position(0,0,s_pos_z-s_thick/2));

        if ( x_slice.isSensitive() ) {
	    		slice_phv.addPhysVolID("layer", myLayerNum++ );
	    		//	    slice_phv.addPhysVolID("slice",s_num);
	  		}

	  		//fg: not needed   slice.setPlacement(slice_phv);

        // Increment Z position of slice.
				s_pos_z -= s_thick;
                                        
        // Increment slice number.
        ++s_num;

      }   // the end of slice loop     

#if DD4HEP_VERSION_GE( 0, 15 )
			caloLayer.outer_nRadiationLengths   = nRadiationLengths
	  						+ (Ecal_fiber_thickness * (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE))/air.radLength();
			caloLayer.outer_nInteractionLengths = nInteractionLengths
	  						+ (Ecal_fiber_thickness * (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE))/air.intLength();
			caloLayer.outer_thickness           = thickness_sum 
	  						+ (Ecal_fiber_thickness * (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE));

			//Only fill the layers information into DDRec after second layer as Mokka Gear.
			caloLayer.thickness = caloLayer.inner_thickness + caloLayer.outer_thickness ;
			if (!isFirstSens) caloData->layers.push_back( caloLayer ) ;
#ifdef VERBOSE
			std::cout<<" caloLayer.distance: "<< caloLayer.distance <<std::endl;
			std::cout<<" caloLayer.inner_nRadiationLengths: "<< caloLayer.inner_nRadiationLengths <<std::endl;
			std::cout<<" caloLayer.inner_nInteractionLengths: "<< caloLayer.inner_nInteractionLengths <<std::endl;
			std::cout<<" caloLayer.inner_thickness: "<< caloLayer.inner_thickness <<std::endl;
			std::cout<<" caloLayer.sensitive_thickness: "<< caloLayer.sensitive_thickness <<std::endl;
			std::cout<<" caloLayer.outer_nRadiationLengths: "<< caloLayer.outer_nRadiationLengths <<std::endl;
			std::cout<<" caloLayer.outer_nInteractionLengths: "<< caloLayer.outer_nInteractionLengths <<std::endl;
			std::cout<<" caloLayer.outer_thickness: "<< caloLayer.outer_thickness <<std::endl;
			std::cout<<" EcalBarrel[2]==>caloLayer.inner_thickness + caloLayer.outer_thickness: "
		 	<< caloLayer.inner_thickness + caloLayer.outer_thickness <<std::endl;
#endif
#endif
	// Init for next double layer
			nRadiationLengths   = radiator_dim_y/(stave_material.radLength())
	  				+ (Ecal_fiber_thickness * (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE))/air.radLength();
			nInteractionLengths = radiator_dim_y/(stave_material.intLength())
	  				+ (Ecal_fiber_thickness * (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE))/air.intLength();
			thickness_sum       = radiator_dim_y
	  				+ (Ecal_fiber_thickness * (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE));
	
			if(radiator_dim_y <= 0) {
	  		stringstream err;
				err << " \n ERROR: The subdetector " << x_det.nameStr() << " geometry parameter -- radiator_dim_y = " << radiator_dim_y ;
	  		err << " \n Please check the radiator material name in the subdetector xml file";
	  		throw runtime_error(err.str());
			}

	// #########################
	// BuildBarrelStructureLayer
	// #########################


			l_dim_x -=  (radiator_dim_y +  Ecal_fiber_thickness * (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE));
			double radiator_dim_x = l_dim_x*2.;

#ifdef VERBOSE
			std::cout << "radiator_dim_x = " << radiator_dim_x << std::endl;
#endif  

			double radiator_dim_z =
	  				Ecal_Barrel_module_dim_z -
	  				2 * Ecal_lateral_face_thickness -
	  				2 * N_FIBERS_W_STRUCTURE * Ecal_fiber_thickness;
	
			string bs_name="bs";
			Box        barrelStructureLayer_box(radiator_dim_x/2.,radiator_dim_z/2.,radiator_dim_y/2.);
			Volume     barrelStructureLayer_vol(det_name+"_"+l_name+"_"+bs_name,barrelStructureLayer_box,stave_material);

			barrelStructureLayer_vol.setVisAttributes(lcdd.visAttributes(x_layer.visStr()));	


      // Increment to next layer Z position.
      l_pos_z -= l_thickness;          

			// Without last W StructureLayer, the last part is Si SD even layer.
			// the last number of  Ecal_nlayers1, Ecal_nlayers2 and  Ecal_nlayers3 is odd.
			//int even_layer = l_num; /////////////////////////////////////////////////////////////////////////*2;
			int even_layer = l_num*2;
			if(even_layer > Ecal_nlayers1 + Ecal_nlayers2 + Ecal_nlayers3) continue;
			//if ( Number_of_Si_Layers_in_Barrel > n_total_layers ) continue;

			double bsl_pos_z = l_pos_z - (radiator_dim_y/2. + Ecal_fiber_thickness * (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE));
			l_pos_z -= (Ecal_fiber_thickness * (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE));

			Position   bsl_pos(0,0,bsl_pos_z);      // Position of the layer.
			PlacedVolume  barrelStructureLayer_phv = mod_vol.placeVolume(barrelStructureLayer_vol,bsl_pos);

			l_dim_x -=  (Ecal_fiber_thickness * (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE));	
			l_pos_z -= (radiator_dim_y + Ecal_fiber_thickness * (N_FIBERS_ALVOULUS + N_FIBERS_W_STRUCTURE));
	
        ++l_num;

    } // end of layer loop
  }
  

    // Set stave visualization.
  if (x_staves)   {
		mod_vol.setVisAttributes(lcdd.visAttributes(x_staves.visStr()));
  }
    
    
    
//====================================================================
// Place ECAL Barrel stave module into the envelope volume
//====================================================================
	double X,Y;
	X = module_thickness * sin(M_PI/4.);
	Y = Ecal_inner_radius + module_thickness / 2.;
    
	for (int stave_id = 1; stave_id <= nsides ; stave_id++) {
		for (int module_id = 1; module_id < 6; module_id++) {
	  double phirot =  (stave_id-1) * dphi - hphi;
	  double module_z_offset =  (2 * module_id-6) * Ecal_Barrel_module_dim_z/2.;
	  
	  // And the rotation in Mokka is right hand rule, and the rotation in DD4hep is clockwise rule
	  // So there is a negitive sign when port Mokka into DD4hep 
	  Transform3D tr(RotationZYX(0,phirot,M_PI*0.5),Translation3D(X*cos(phirot)-Y*sin(phirot),
								      X*sin(phirot)+Y*cos(phirot),
								      module_z_offset));
	  PlacedVolume pv = envelope.placeVolume(mod_vol,tr);
	  pv.addPhysVolID("module",module_id);
	  pv.addPhysVolID("stave",stave_id);
	  DetElement sd = (module_id==0&&stave_id==0) ? stave_det : stave_det.clone(_toString(module_id,"module%d")+_toString(stave_id,"stave%d"));
	  sd.setPlacement(pv);
	  sdet.add(sd);
	  } 
	}
    
    // Set envelope volume attributes.
    envelope.setAttributes(lcdd,x_det.regionStr(),x_det.limitsStr(),x_det.visStr());
    
    sdet.addExtension< DDRec::LayeringExtension >( layeringExtension ) ;
    sdet.addExtension< DDRec::SubdetectorExtension >( subDetExtension ) ;
    sdet.addExtension< DDRec::LayeredCalorimeterData >( caloData ) ; 


    return sdet;
}

DECLARE_DETELEMENT(SEcal04Hybrid_Barrel,create_detector)
