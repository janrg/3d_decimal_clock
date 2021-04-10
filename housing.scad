
epsilon = 0.0001;

module copies(x=[0], y=[0], z=[0], theta=[0]) {
  for (X = x)
    for (Y = y)
      for (Z = z)
        for (Theta = theta)
          rotate(Theta) translate([X, Y, Z]) children();
}

module ccube(size, center="xy") {
  translate([
    len(search("x", center)) > 0 ? -size[0] / 2 : 0,
    len(search("y", center)) > 0 ? -size[1] / 2 : 0,
    len(search("z", center)) > 0 ? -size[2] / 2 : 0,
  ])
    cube(size, center=false);
}


module led_module_backplate() {
  difference() {
    union() {
      ccube([32 + epsilon, 32 + epsilon, 5]);
      copies(x=[-13.2, 13.2], y=[-10.1, 10.1])
          cylinder(h=8, r=2.8 - epsilon, $fn=50);
    }
    copies(x=[-16, 16])
      ccube([10, 13, 20], center="xyz");
    copies(x=[-13.2, 13.2], y=[-10.1, 10.1]) {
      cylinder(h=20, r=1.7, $fn=50, center=true);
      rotate(30)
        cylinder(h=5.2, r=5.61*0.5/cos(30), $fn=6, center=true);
    }
  }
}

module dummy_arduino() {
  ccube([44.5, 18.6, 2]);
  translate([-19.5, 0, 1.8])
    ccube([9.5, 8, 4.4]);
  translate([-2.25, 0, -1.6])
    ccube([40, 13, 6]);
}

module dummy_ds3231() {
  difference() {
      ccube([39, 22.5, 1.8]);
    copies(x=[-19.5, 19.5], y=[-11.25, 11.25])
      cylinder(h=10, r=1, $fn=50, center=true);
  }
  translate([1, 0, 0])
    ccube([25, 22.5, 11]);
  translate([0, 0, -1.8])
    ccube([19, 22.5, 4.2]);
  translate([-1.5, 0, -1.8])
    ccube([36, 16.5, 4.2]);
}

module ds3231_mount_test() {
  difference() {
    union() {
      copies(x=[-19], y=[-10.75, 10.75], z=[-1.2])
        cube([5, 5, 6], center=true);
      translate([19, 0, -1.2])
        cube([5, 26.5, 6], center=true);
      translate([0, 0, -4])
        cube([43, 26.5, 2], center=true);
    }
    dummy_ds3231_old();
    translate([0, 0, -5])
      cube([33, 16.5, 5], center=true);
  }
}

module dummy_dip_switch() {
  ccube([4.5, 6.4, 6.4]);
  copies(x=[3.5], y=[-3.5, 3.5])
    ccube([6, 2, 6.4]);
  translate([-7, 0, 3.2])
    rotate([0, 90, 0])
      cylinder(h=5, r=2, $fn=50);
}

module dip_switch_mount_test() {
  difference() {
    cube([8.5, 12, 10], center=true);
    translate([0, 0, 2]) dummy_dip_switch_old();
    translate([-3, 0, 3.9]) cube([4, 4, 4], center=true);
  }
}

module cutouts() {
  translate([-45.1, 0, 13])
    rotate([180, 0, 0])
      dummy_arduino();
  translate([29, 0, 12.8])
    rotate([180, 0, 0])
      dummy_ds3231();
  translate([0, 17, -epsilon])
    rotate(-90)
      dummy_dip_switch();
  copies(x=[-64.75, 64.75], y=[-16.75, 16.75]) {
    translate([0, 0, -epsilon])
      cylinder(h=13, r=1.7, $fn=50);
    translate([0, 0, 5])
      rotate(30)
        cylinder(h=3, r=5.8*0.5/cos(30), $fn=6);
  }
  translate([0, 20, -epsilon])
    cube([4, 4, 6], center=true);
}

module custom_bridge_supports() {
  // arduino
  translate([-23.3, 0, 0]) {
    difference() {
      ccube([10, 25, 11.2]);
      translate([0, 0, 0.2])
        ccube([6.8, 26, 10.8]);
    }
  }
  translate([-66.75, 0, 13])
    ccube([5, 15, 0.2]);
  // DS3231
  translate([48, 0, 0]) {
    difference() {
      ccube([10, 26.5, 11.2]);
      translate([0, 0, 0.2])
        ccube([6.8, 27, 10.8]);
    }
  }
  translate([29, 0, 0])
    copies(x=[-19], y=[-10.75, 10.75]) {
      difference() {
        ccube([5, 10, 13]);
        translate([0, 0, 0.2])
          ccube([6, 6.8, 12.6]);
      }
      translate([0, 0, 9])
        ccube([5, 10, 0.2]);
    }
  // backplate cutouts
  translate([0, 0, 17]) ccube([130, 35, 0.2]);
  // backplate nut cutouts
  copies(y=[-10, 10], z=[19.6])
    ccube([130, 7, 0.2]);
  // corner nut cutouts
  copies(x=[-64.75, 64.75], y=[-16.75, 16.75], z=[8])
    cylinder(h=0.2, r=4);
}

module frame() {
  difference() {
    union() {
      difference() {
        ccube([138.5, 42.5, 38]);
        translate([0, 0, -1]) ccube([128.5, 32.25, 40]);
      }
      translate([0, 0, 17]) difference() {
        ccube([130, 34, 5]);
        translate([0, 0, -1]) ccube([128, 32, 7]);
      }
      copies(x=[-48, -16, 16, 48], z=[17])
        led_module_backplate();
      // screw bulges
      copies(x=[-64.25, 64.25], y=[-16.25, 16.25])
        cylinder(h=22, r=4, $fn=50);
      // arduino mount
      translate([-23.3, 0, 11 + epsilon])
        ccube([5, 25, 10]);
      // DS3231 mount
      copies(x=[10], y=[-10.75, 10.75], z=[9 + epsilon])
        ccube([5, 5, 12]);
      translate([48, 0, 11 + epsilon])
        ccube([5, 26.5, 10]);
      // dip switch bulge
      translate([0, 15, 0]) 
        ccube([12, 4.5, 10]);
    }
    cutouts();
  }
  custom_bridge_supports();
}

module lid() {
  difference() {
    ccube([138.5, 42.5, 5]);
    copies(x=[-64.75, 64.75], y=[-16.75, 16.75]) {
      translate([0, 0, -1]) cylinder(h=7, r=1.7, $fn=50);
      translate([0, 0, -1])
        cylinder(h=4, r=3, $fn=50);
    }
  }
  translate([-23.3, 0, 0])
    ccube([5, 25, 15.8]);
  translate([48, 0, 0])
    ccube([5, 26.5, 15.8]);
  
  difference() {
    translate([0, 20.25, 0])
      ccube([3.8, 2, 8.4]);
    translate([0, 22, 8.2])
      rotate([90, 0, 0])
        cylinder(h=5, r=2, $fn=50);
  }
  copies(x=[-35, 35], y=[-15.1, 15.1])
    ccube([10, 2, 7]);
  copies(x=[-63.1, 63.1])
    ccube([2, 10, 7]);
  translate([0, 0, 3])
    ccube([138.5, 42.5, 0.2]);
}

rotate(180) frame();
rotate(180) lid();
