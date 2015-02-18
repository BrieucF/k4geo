// Silicon Tracker Barrel implementation for the CLIC detector
//====================================================================
//--------------------------------------------------------------------
//
//  Author     : N. Nikiforou (forked from SiTrackerBarrel_geo.cpp
//
//====================================================================
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"

using namespace std;
using namespace DD4hep;
using namespace DD4hep::Geometry;

static Ref_t create_detector(LCDD& lcdd, xml_h e, SensitiveDetector sens)  {
    typedef vector<PlacedVolume> Placements;
    xml_det_t   x_det     = e;
    Material    air       = lcdd.air();
    int         det_id    = x_det.id();
    string      det_name  = x_det.nameStr();
    DetElement  sdet       (det_name,det_id);
    Assembly    assembly   (det_name);
    map<string, Volume>    volumes;
    map<string, Placements>  sensitives;
    PlacedVolume pv;
    
    sens.setType("tracker");
    
    //NOTE modules are what is defined in compact. Later we call a "module" as a "sensor".
    for(xml_coll_t mi(x_det,_U(module)); mi; ++mi)  {
        xml_comp_t x_mod  = mi;
        xml_comp_t m_env  = x_mod.child(_U(module_envelope));
        string     m_nam  = x_mod.nameStr();
        Volume     m_vol(m_nam,Box(m_env.width()/2,m_env.length()/2,m_env.thickness()/2),air);
        int        ncomponents = 0, wafer_number = 0;
        
        if ( volumes.find(m_nam) != volumes.end() )   {
            printout(ERROR,"SiTrackerBarrel","Logics error in building modules.");
            throw runtime_error("Logics error in building modules.");
        }
        volumes[m_nam] = m_vol;
        m_vol.setVisAttributes(lcdd.visAttributes(x_mod.visStr()));
        for(xml_coll_t ci(x_mod,_U(module_component)); ci; ++ci, ++ncomponents)  {
            xml_comp_t x_comp = ci;
            xml_comp_t x_pos  = x_comp.position(false);
            xml_comp_t x_rot  = x_comp.rotation(false);        
            string     c_nam  = _toString(ncomponents,"component%d");
            Box        c_box(x_comp.width()/2,x_comp.length()/2,x_comp.thickness()/2);
            Volume     c_vol(c_nam,c_box,lcdd.material(x_comp.materialStr()));
            
            if ( x_pos && x_rot ) {
                Position    c_pos(x_pos.x(0),x_pos.y(0),x_pos.z(0));
                RotationZYX c_rot(x_rot.z(0),x_rot.y(0),x_rot.x(0));
                pv = m_vol.placeVolume(c_vol, Transform3D(c_rot,c_pos));
            }
            else if ( x_rot ) {
                pv = m_vol.placeVolume(c_vol,RotationZYX(x_rot.z(0),x_rot.y(0),x_rot.x(0)));
            }
            else if ( x_pos ) {
                pv = m_vol.placeVolume(c_vol,Position(x_pos.x(0),x_pos.y(0),x_pos.z(0)));
            }
            else {
                pv = m_vol.placeVolume(c_vol);
            }
            c_vol.setRegion(lcdd, x_comp.regionStr());
            c_vol.setLimitSet(lcdd, x_comp.limitsStr());
            c_vol.setVisAttributes(lcdd, x_comp.visStr());
            if ( x_comp.isSensitive() ) {
//                 pv.addPhysVolID("wafer",wafer_number++); //FIXME not needed (no ID spec wafer )
                c_vol.setSensitiveDetector(sens);
                sensitives[m_nam].push_back(pv);
            }
        }
    }
    for(xml_coll_t li(x_det,_U(layer)); li; ++li)  {
        xml_comp_t x_layer  = li;
        xml_comp_t x_barrel = x_layer.child(_U(barrel_envelope));
        xml_comp_t x_layout = x_layer.child(_U(rphi_layout));
        xml_comp_t z_layout = x_layer.child(_U(z_layout));      // Get the <z_layout> element.
        int        lay_id   = x_layer.id();
        string     m_nam    = x_layer.moduleStr();
        string     lay_nam  = _toString(x_layer.id(),"layer%d");
        Tube       lay_tub   (x_barrel.inner_r(),x_barrel.outer_r(),x_barrel.z_length()/2);
        Volume     lay_vol   (lay_nam,lay_tub,air);         // Create the layer envelope volume.
        double     phi0     = x_layout.phi0();              // Starting phi of first sensor.
        double     phi_tilt = x_layout.phi_tilt();          // Phi tilt of a sensor.
        double     rc       = x_layout.rc();                // Radius of the sensor center.
        int        nphi     = x_layout.nphi();              // Number of sensors in phi.
        double     rphi_dr  = x_layout.dr();                // The delta radius of every other sensor.
        double     phi_incr = (M_PI * 2) / nphi;            // Phi increment for one sensor.
        double     phic     = phi0;                         // Phi of the sensor center.
        double     z0       = z_layout.z0();                // Z position of first sensor in phi.
        double     nz       = z_layout.nz();                // Number of sensors to place in z.
        double     z_dr     = z_layout.dr();                // Radial displacement parameter, of every other sensor.
        Volume     m_env    = volumes[m_nam];
        DetElement lay_elt(sdet,_toString(x_layer.id(),"layer%d"),lay_id);
        Placements& waferVols = sensitives[m_nam];
        
        // Z increment for sensor placement along Z axis.
        // Adjust for z0 at center of sensor rather than
        // the end of cylindrical envelope.
        double z_incr   = nz > 1 ? (2.0 * z0) / (nz - 1) : 0.0;
        // Starting z for sensor placement along Z axis.
        double sensor_z = -z0;
        int module_idx =0;  
        // Loop over the number of sensors in phi.
        for (int ii = 0; ii < nphi; ii++)        {
            double dx = z_dr * std::cos(phic + phi_tilt);        // Delta x of sensor position.
            double dy = z_dr * std::sin(phic + phi_tilt);        // Delta y of sensor position.
            double  x = rc * std::cos(phic);                     // Basic x sensor position.
            double  y = rc * std::sin(phic);                     // Basic y sensor position.
            
            // Loop over the number of sensors in z.
            //Create stave FIXME disable for now
            string module_name = _toString(module_idx,"module%d");
            //       DetElement module_elt(lay_elt,module_name,module_idx);
            int sensor_idx = 0;
            
            for (int j = 0; j < nz; j++)          {
                string sensor_name = _toString(sensor_idx,"sensor%d");
                
                //FIXME
                sensor_name = module_name + sensor_name;
                
                DetElement sens_elt(lay_elt,sensor_name,sensor_idx);
                // Module PhysicalVolume.
                //         Transform3D tr(RotationZYX(0,-((M_PI/2)-phic-phi_tilt),M_PI/2),Position(x,y,sensor_z));
                //NOTE (Nikiforos, 26/08 Rotations needed to be fixed so that component1 (silicon) is on the outside
                Transform3D tr(RotationZYX(0,((M_PI/2)-phic-phi_tilt),-M_PI/2),Position(x,y,sensor_z));
                
                //FIXME
                pv = lay_vol.placeVolume(m_env,tr);
                pv.addPhysVolID(_U(module), module_idx);
                pv.addPhysVolID(_U(sensor), sensor_idx);
                sens_elt.setPlacement(pv);
                for(size_t ic=0; ic<waferVols.size(); ++ic)  {
                    PlacedVolume wafer_pv = waferVols[ic];
                    DetElement comp_elt(sens_elt,wafer_pv.volume().name(),sensor_idx);
                    comp_elt.setPlacement(wafer_pv);
                }
                
                /// Increase counters etc.
                sensor_idx++;
                // Adjust the x and y coordinates of the sensor.
                x += dx;
                y += dy;
                // Flip sign of x and y adjustments.
                dx *= -1;
                dy *= -1;
                // Add z increment to get next z placement pos.
                sensor_z += z_incr;
            }
            module_idx++;
            phic     += phi_incr;      // Increment the phi placement of sensor.
            rc       += rphi_dr;       // Increment the center radius according to dr parameter.
            rphi_dr  *= -1;            // Flip sign of dr parameter.
            sensor_z  = -z0;           // Reset the Z placement parameter for sensor.
        }
        // Create the PhysicalVolume for the layer.
        pv = assembly.placeVolume(lay_vol); // Place layer in mother
        pv.addPhysVolID("layer", lay_id);       // Set the layer ID.
        lay_elt.setAttributes(lcdd,lay_vol,x_layer.regionStr(),x_layer.limitsStr(),x_layer.visStr());
        lay_elt.setPlacement(pv);
    }
    sdet.setAttributes(lcdd,assembly,x_det.regionStr(),x_det.limitsStr(),x_det.visStr());
    assembly.setVisAttributes(lcdd.invisible());
    pv = lcdd.pickMotherVolume(sdet).placeVolume(assembly);
    pv.addPhysVolID("system", det_id);      // Set the subdetector system ID.
    pv.addPhysVolID("barrel", 0);           // Flag this as a barrel subdetector.
    sdet.setPlacement(pv);
    return sdet;
}

DECLARE_DETELEMENT(TrackerBarrel_o1_v01,create_detector)
