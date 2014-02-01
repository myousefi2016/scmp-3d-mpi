#include "fillGhostLayers.h"

// fill ghost layers in the macroscopic variable buffers ( rho, u, v, w )

void fillGhostLayersMacVar(const int       nn,              // ghost layer thickness
                           const int       LX,              // number of nodes along X (local for this MPI process)
                           const int       LY,              // number of nodes along Y (local for this MPI process)
                           const int       LZ,              // number of nodes along Z (local for this MPI process)
                           const int       myid,            // MPI process id or rank
                           const MPI_Comm  CART_COMM,       // Cartesian communicator
                           const int       nbr_WEST,        // neighboring MPI process to my west
                           const int       nbr_EAST,        // neighboring MPI process to my east
                           const int       nbr_SOUTH,       // neighboring MPI process to my south
                           const int       nbr_NORTH,       // neighboring MPI process to my north
                           const int       nbr_BOTTOM,      // neighboring MPI process to my bottom
                           const int       nbr_TOP,         // neighboring MPI process to my top
                                 double    *rho,            // density
                                 double    *u,              // velocity (x-component)
                                 double    *v,              // velocity (y-component)
                                 double    *w)              // velocity (z-component)
{

    exchangeDBL  (nn,              // ghost layer thickness
                  LX,              // number of nodes along X (local for this MPI process)
                  LY,              // number of nodes along Y (local for this MPI process)
                  LZ,              // number of nodes along Z (local for this MPI process)
                  myid,            // MPI process id or rank
                  CART_COMM,       // Cartesian communicator
                  nbr_WEST,        // neighboring MPI process to my west
                  nbr_EAST,        // neighboring MPI process to my east
                  nbr_SOUTH,       // neighboring MPI process to my south
                  nbr_NORTH,       // neighboring MPI process to my north
                  nbr_BOTTOM,      // neighboring MPI process to my bottom
                  nbr_TOP,         // neighboring MPI process to my top
                  rho);            // local density buffer (including ghost cells)

    exchangeDBL  (nn,              // ghost layer thickness
                  LX,              // number of nodes along X (local for this MPI process)
                  LY,              // number of nodes along Y (local for this MPI process)
                  LZ,              // number of nodes along Z (local for this MPI process)
                  myid,            // MPI process id or rank
                  CART_COMM,       // Cartesian communicator
                  nbr_WEST,        // neighboring MPI process to my west
                  nbr_EAST,        // neighboring MPI process to my east
                  nbr_SOUTH,       // neighboring MPI process to my south
                  nbr_NORTH,       // neighboring MPI process to my north
                  nbr_BOTTOM,      // neighboring MPI process to my bottom
                  nbr_TOP,         // neighboring MPI process to my top
                  u);              // local x-component of velocity (including ghost cells)

    exchangeDBL  (nn,              // ghost layer thickness
                  LX,              // number of nodes along X (local for this MPI process)
                  LY,              // number of nodes along Y (local for this MPI process)
                  LZ,              // number of nodes along Z (local for this MPI process)
                  myid,            // MPI process id or rank
                  CART_COMM,       // Cartesian communicator
                  nbr_WEST,        // neighboring MPI process to my west
                  nbr_EAST,        // neighboring MPI process to my east
                  nbr_SOUTH,       // neighboring MPI process to my south
                  nbr_NORTH,       // neighboring MPI process to my north
                  nbr_BOTTOM,      // neighboring MPI process to my bottom
                  nbr_TOP,         // neighboring MPI process to my top
                  v);              // local y-component of velocity (including ghost cells)

    exchangeDBL  (nn,              // ghost layer thickness
                  LX,              // number of nodes along X (local for this MPI process)
                  LY,              // number of nodes along Y (local for this MPI process)
                  LZ,              // number of nodes along Z (local for this MPI process)
                  myid,            // MPI process id or rank
                  CART_COMM,       // Cartesian communicator
                  nbr_WEST,        // neighboring MPI process to my west
                  nbr_EAST,        // neighboring MPI process to my east
                  nbr_SOUTH,       // neighboring MPI process to my south
                  nbr_NORTH,       // neighboring MPI process to my north
                  nbr_BOTTOM,      // neighboring MPI process to my bottom
                  nbr_TOP,         // neighboring MPI process to my top
                  w);              // local z-component of velocity (including ghost cells)
}
