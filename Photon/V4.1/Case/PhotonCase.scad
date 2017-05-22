$fn=60;

pcbLength = 55;
pcbWidth = 32;
pcbHeight = 0.8;

pcbXOffset = 2.5;
pcbYOffset = 2.5;
pcbZOffset = 5;

// ==========================================================
// Models
// ==========================================================

module showPcb() {
    roundedCube(pcbWidth, pcbLength, pcbHeight , 4);
    
    // Hole
    translate([4,4,-2]) {
        #cylinder(d=3, h=4);
    }
    
    // Hole
    translate([4+24,4,-2]) {
        cylinder(d=3, h=4);
    }
    
    // Thermal cutout
    translate([7.6,42.1,-2]) {
        cube([pcbWidth-(7.6 * 2),1.6,4]);
    }
    
    // USB
    translate([(pcbWidth-12)/2,-15,-3]) {
        cube([12,15,4.5]);
    }
    
    // Photon
    translate([5.7,5,pcbHeight]) {
        // Height the max of the PCB - USB plug + aerial
        cube([20,37,4]);
    }
    
    // JST for battery
    translate([6,17,-6]) {
        // Height the max of the PCB - USB plug + aerial
        cube([20,10,6]);
    }
    
    // LED
    translate([20,48,-1]) {
        // Height the max of the PCB - USB plug + aerial
        cube([11,5,1]);
    }
    
    translate([28,50,-1]) {
        // Height the max of the PCB - USB plug + aerial
        #cylinder(d=2, h=20);
    }
}

// ==========================================================
// Helpers
// ==========================================================

module roundedCube(width, height, depth, cornerDiameter) {
//cornerDiameter = 5;
cornerRadius = cornerDiameter/2;
    
    translate([cornerDiameter/2,0,0]) {
        cube([width-cornerDiameter, height, depth]);
    }
    
    translate([0,cornerDiameter/2,0]) {
        cube([width, height-cornerDiameter, depth]);
    }
    
    translate([cornerRadius,cornerRadius,0]) {
        cylinder(d=cornerDiameter, h=depth);
    }
    
    translate([width-cornerRadius,cornerRadius,0]) {
        cylinder(d=cornerDiameter, h=depth);
    }
    
    translate([cornerRadius,height-cornerRadius,0]) {
        cylinder(d=cornerDiameter, h=depth);
    }
    
    translate([width-cornerRadius,height-cornerRadius,0]) {
        cylinder(d=cornerDiameter, h=depth);
    }
}

// ==========================================================
// PCB Mounts
// ==========================================================

module pcbMountingPads() {
    translate([pcbXOffset, pcbYOffset,0]) {
        pcbMountingPin(4,4);
        pcbMountingPin(4+24,4);
    }
}

module pcbMountingPad(x,y) {
    translate([x,y,0]) {
        difference() {
            cylinder(d=7, h=pcbZOffset);
            cylinder(d=1, h=4);
        }
    }
}

module pcbMountingPin(x,y) {
    translate([x,y,0]) {
        cylinder(d=7, h=pcbZOffset);
        cylinder(d1=3.1, d2=2.8, h=pcbZOffset+4);
    }
}

// A Support "pad" for the back of the PCB.
// use the slot in the PCB to help stabalise things.
module pcbSupport() {
    translate([pcbXOffset, pcbYOffset,0]) {
        // Add a pad
        // 9mm from pCB edge.
        translate([-pcbXOffset,40,0]) {
            cube([pcbWidth + (pcbXOffset*2),2.5,pcbZOffset-0.1]);
        }
            
        // add a riser
        translate([8,42.25,0]) {
            cube([16,1.4,pcbZOffset+4]);
        }
    }
}

module pcbMountingReceptors() {
    translate([pcbXOffset, pcbYOffset,-1]) {
        pcbMountingReceptor(4,4);
        pcbMountingReceptor(4+24,4);
    }
}

module pcbMountingReceptor(x,y) {
    translate([x,y,0]) {
        difference() {
            cylinder(d=7, h=mainBodyHeight+1);
            cylinder(d=3.2, h=mainBodyHeight+1-2);
        }
    }
}

// ==========================================================

mainBodyHeight = 6.5;
    //mainBodyHeight = 1;

module usbCutout() {
usbWidth  = 12.5;
xPosition = (pcbWidth / 2) - (usbWidth / 2) + pcbXOffset;
    
    // USB plug pretrudes 3.3mm below the PCB 
    translate([xPosition, -0.1,pcbZOffset - 3.5]) {
        // Ensure no top is put onto the USB
       # cube([usbWidth,3,5.5]);
    }
}

module photodiodeTopHole(height) {
    translate([22, 50,0]) {
        difference() {
            union() {
                cylinder(d=6, h=height);
            }
            union() {
                cylinder(d=4, h=height);
            }
        }
    }
}


module body() {   
    
    difference() {
        union() {
            roundedCube(pcbWidth + pcbXOffset * 2, 63, mainBodyHeight, 8) ;
        }
        union() {
            translate([1.5,1.5,1.5]) {
                // 1mm difference between wall nand pcbXOffset
                // double that so it's either side.
                roundedCube(pcbWidth + 2, 60, mainBodyHeight, 4) ;
            }
            
            usbCutout();
            
            // Vent holes for sensors
            translate([pcbXOffset,pcbYOffset,-0.1]) {
                for (i = [2 : 3 : 30]) {
                    translate([i, 45,0]) {
                        #cube([1.5, 12,  2]);
                    }
                }
            }
            
            // Vent holes for Photon heat
            translate([pcbXOffset,pcbYOffset,-0.1]) {
                for (i = [2 : 3 : 30]) {
                    translate([i, 8,0]) {
                        // Shorter to allow for a cutout for the JST
                        #cube([1.5, 8,  2]);
                    }
                }
            }
            
            // Photodiode hole.
            translate([pcbXOffset,pcbYOffset,-0.1]) {
                translate([22, 50,0]) {
                    #cylinder(d=4, h=mainBodyHeight+1);
                }
            }
        }
    }
    
    pcbMountingPads();
    pcbSupport();
    
    translate([pcbXOffset,pcbYOffset,0]) {
        photodiodeTopHole(pcbZOffset-1);
    }
    
    // End catch to get the lid
    translate([7.5,63-3.5,0]) {
        difference() {
            union() {
                cube([pcbWidth-10,2, mainBodyHeight + mainBodyHeight/2]);
            }
            union() {
                translate([0,2.5,mainBodyHeight ]) {
                    rotate([45, 0,0]) {
                        cube([pcbWidth-10,2, 2]);
                    }
                }
                translate([0,2.5,mainBodyHeight+2 ]) {
                    rotate([45, 0,0]) {
                        cube([pcbWidth-10,2, 2]);
                    }
                }
            }
        }
    }
}


module lid() {

    difference() {
        union() {
            roundedCube(pcbWidth + pcbXOffset * 2, 63, mainBodyHeight, 8) ;
        }
        union() {
            translate([1.5,1.5,-0.1]) {
                // 1mm difference between wall nand pcbXOffset
                // double that so it's either side.
                roundedCube(pcbWidth + 2, 60, mainBodyHeight-1.6, 4 ) ;
            }
            
            // LED hole
            translate([pcbXOffset,pcbYOffset,0]) {
                translate([27.75, 50,0]) {
                    #cylinder(d=4, h=mainBodyHeight+1);
                }
            }
            
            // Photodiode hole.
            translate([pcbXOffset,pcbYOffset,0]) {
                translate([22, 50,0]) {
                    cylinder(d=4, h=mainBodyHeight+1);
                }
            }
            
            // Vent holes.
            translate([pcbXOffset,pcbYOffset,0]) {
                for (i = [2 : 3 : 18]) {
                    translate([i, 45,0]) {
                        #cube([1.5, 10,  mainBodyHeight+1]);
                    }
                }
            }
            
            // Vent holes for Photon heat
            translate([pcbXOffset,pcbYOffset,0.1]) {
                for (i = [2 : 3 : 30]) {
                    translate([i, 8,0]) {
                        #cube([1.5, 6,  mainBodyHeight+1]);
                    }
                }
            }
            
            translate([pcbXOffset,pcbYOffset,0.1]) {
                for (i = [2 : 3 : 30]) {
                    translate([i, 20,0]) {
                        #cube([1.5, 6,  mainBodyHeight+1]);
                    }
                }
            }
        }
    }
    
    // Photodiode hole (in addition to the opening in the top case)
    translate([pcbXOffset,pcbYOffset,0]) {
        photodiodeTopHole(mainBodyHeight);
    }
    
    difference() {
        union() {
            pcbMountingReceptors();
        }
        union() {
            translate([pcbXOffset,pcbYOffset,-1.5]) {
                translate([5,5,0]) {
                    // Now cut out corners of the pads where the Photon PCB is.
                    #cube([22,30,3]);
                }
            }
        }
    }
    
    // End catch for the base.
    translate([7.5,63-3.5,0]) {
        difference() {
            union() {
                translate([0,2.5,0]) {
                    rotate([45, 0,0]) {
                        cube([pcbWidth-10,1.9, 1.9]);
                    }
                }
                
            }
            union() {
                translate([0,3,0]) {
                    cube([pcbWidth-10,2, 5]);
                }
            }
        }
    }
}


body();

translate([0,0,10]) {
    lid();
}
    

translate([pcbXOffset,pcbYOffset,pcbZOffset]) {
    %showPcb();
}