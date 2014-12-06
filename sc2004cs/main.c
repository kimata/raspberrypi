#include <stdio.h>
#include "rp_gpio.h"
#include "sc2004c_ctrl.h"

int main(int argc,char *argv[])
{
  sc2004c_gpio_assign_t assign;
  assign.rs = 21;
  assign.en = 22;
  assign.d4 = 24;
  assign.d5 = 25;
  assign.d6 = 26;
  assign.d7 = 27;

  sc2004c_init(&assign);
  sc2004c_print("SC2004CS control");
  sc2004c_set_line(1);
  sc2004c_print("±²³´µ");
  
  return 0;
}

// Local Variables:
// coding: shift_jis-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
