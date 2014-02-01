#include "exchangeInfo.h"

/**
MPI communication routine for exchanging (double-precision) values 
of particle distribution functions across the boundaries
between different MPI processes (or MPI ranks)

After calling this function, values in the ghost layers for {f0, f1, ..., f18}
get updated using values from neighboring MPI processes
*/
void exchangePDF (const int      nn,                // number of ghost cell layers
                  const int      Q,                 // number of LBM streaming directions
                  const int      MX,                // number of voxels along X in this process
                  const int      MY,                // number of voxels along Y in this process
                  const int      MZ,                // number of voxels along Z in this process
                  const int      myid,              // my process id
                  const MPI_Comm CART_COMM,         // Cartesian topology communicator
                  const int      nbr_WEST,          // process id of my western neighbor
                  const int      nbr_EAST,          // process id of my eastern neighbor
                  const int      nbr_SOUTH,         // process id of my southern neighbor
                  const int      nbr_NORTH,         // process id of my northern neighbor
                  const int      nbr_BOTTOM,        // process id of my bottom neighbor
                  const int      nbr_TOP,           // process id of my top neighbor
                     double      *PDF4d)             // pointer to the 4D array being exchanged (of type double)
{
    MPI_Status status;

    const int MXP = nn+MX+nn;  // padded voxels along X
    const int MYP = nn+MY+nn;  // padded voxels along Y
    const int MZP = nn+MZ+nn;  // padded voxels along Z

    // regular voxels + voxels in the ghost layer
    const int PADDED_VOXELS = MXP*MYP*MZP;

    // allocate a 3D array for storing f(a)
    // ghost layers are included in this 3D array
    double *PDF3d = new double[PADDED_VOXELS]; 

    // loop for all PDF directions
    for (int a = 0; a < Q; a++)
    {
        // loop over all voxels in this MPI process, including ghost layers 
        for(int i = 0; i < MXP; i++) {
            for(int j = 0; j < MYP; j++) {
                for(int k = 0; k < MZP; k++) {

                    // natural index for fa(i,j,k) in PDF3d
                    int index_3d = i + j*MXP + k*MXP*MYP;

                    // natural index for f(i,j,k,a) in PDF4d
                    int index_4d = a + (index_3d * Q);

                    // PDF3d <---- PDF4d(a)
                    PDF3d[index_3d] = PDF4d[index_4d];
                }
            }
        }
 
        // 
        MPI_Datatype stridex;
        MPI_Type_vector( (MY+nn+nn)*(MZ+nn+nn), 1, MX+nn+nn, MPI_DOUBLE, &stridex);
        MPI_Type_commit( &stridex);

        //
        MPI_Datatype stridey;
        MPI_Type_vector( MZ+nn+nn, MX+nn+nn, (MY+nn+nn)*(MX+nn+nn), MPI_DOUBLE, &stridey);
        MPI_Type_commit( &stridey);

        // total number of values in a XY plane (contiguous values)
        //
        // example layout for the case nn = 1 (1 layer of ghost cells)
        //          
        //
        //         0,MY+1  1,MY+1  2,MY+1  3,MY+1  ...    MX,MY+1 MX+1,MY+1
        // 
        //              +-----------------------      ----------+     
        //              |                                       |
        //         0,MY |  1,MY    2,MY    3,MY    ...    MX,MY | MX+1,MY
        //              |                                       |
        //
        //
        //
        //
        //
        //         0,2  |  2,2     2,2     3,2     ...    MX,2  | MX+1,2
        //              |                                       |
        //              |                                       |
        //              |                                       |
        //         0,1  |  1,1     2,1     3,1     ...    MX,1  | MX+1,1
        //              |                                       |
        //              +-----------------------       ---------+     
        //
        //         0,0     1,0     2,0     3,0     ...    MX,0    MX+1,0
        //
        //
        int no_xy = MXP*MYP;  // we are only exchanging one PDF at a time


    // loop over the number of ghost layers
    for(int i = 0; i < nn; i++)
    {
        // I am sending PDF3d data to the process nbr_TOP and receiving PDF3d data from the process nbr_BOTTOM
        {
            //                                                 x   x   x   x   x   x
            // send the topmost (non-ghost) layer of data        +---------------+
            // receive this data into the ghost cell layer     S | S   S   S   S | S  --- send to nbr_TOP
            //                                                   |               |
            //                  ^                              x | o   o   o   o | x
            //                  |  Z-axis                        |               |
            //                  |                              x | o   o   o   o | x
            //                  |                                +---------------+
            //                  |                              R   R   R   R   R   R  --- recv from nbr_BOTTOM

            // SEND to top
            int sx = 0;
            int sy = 0;
            int sz = nn + (MZ-1) - i;

            // RECV from bottom
            int rx = 0;
            int ry = 0;
            int rz = (nn - 1) - i;

            int send = sx + sy * MXP + sz * MXP*MYP;  // send the topmost (non-ghost) layer of data
            int recv = rx + ry * MXP + rz * MXP*MYP;  // receive data into the bottom ghost cell layer

            MPI_Sendrecv(&PDF3d[send],       // send buffer (points to the starting address of the data chunk)
                         no_xy,              // number of elements to be sent
                         MPI_DOUBLE,         // type of elements
                         nbr_TOP,            // destination (where the data is going)
                         111,                // tag
                         &PDF3d[recv],       // receive buffer (points to the starting address of the data chunk)
                         no_xy,              // number of elements received
                         MPI_DOUBLE,         // type of elements
                         nbr_BOTTOM,         // source (where the data is coming from)
                         111,                // tag
                         CART_COMM,          // MPI Communicator used for this Sendrecv
                         &status);           // MPI status
        }

        // I am sending PDF3d data to the process nbr_BOTTOM and receiving PDF3d data from the process nbr_TOP
        {
            //                                                 R   R   R   R   R   R  --- recv from nbr_TOP
            // send the topmost (non-ghost) layer of data        +---------------+
            // receive this data into the ghost cell layer     x | o   o   o   o | x
            //                                                   |               |
            //                  ^                              x | o   o   o   o | x
            //                  |  Z-axis                        |               |
            //                  |                              S | S   S   S   S | S  --- send to nbr_BOTTOM
            //                  |                                +---------------+
            //                  |                              x   x   x   x   x   x

            // SEND to bottom
            int sx = 0;
            int sy = 0;
            int sz = nn + i;

            // RECV from top
            int rx = 0;
            int ry = 0;
            int rz = nn + MZ + i;

            int send = sx + sy * MXP + sz * MXP*MYP; // send the bottommost (non-ghost) layer of data
            int recv = rx + ry * MXP + rz * MXP*MYP; // receive data into the top ghost cell layer

            MPI_Sendrecv(&PDF3d[send],       // send buffer (points to the starting address of the data chunk) 
                         no_xy,              // number of elements to be sent
                         MPI_DOUBLE,         // type of elements
                         nbr_BOTTOM,         // destination (where the data is going)
                         222,                // tag
                         &PDF3d[recv],       // receive buffer (points to the starting address of the data chunk)
                         no_xy,              // number of elements received
                         MPI_DOUBLE,         // type of elements
                         nbr_TOP,            // source (where the data is coming from)
                         222,                // tag
                         CART_COMM,          // MPI Communicator used for this Sendrecv
                         &status);           // MPI status
        }

        // I am sending PDF3d data to the process nbr_EAST and receiving PDF3d data from process nbr_WEST
        {
            //                                                 R   x   x   x   S   x
            // send the eastmost (non-ghost) layer of data       +---------------+
            // receive data into the west ghost cell layer     R | o   o   o   S | x     S --- send to nbr_EAST
            //                                                   |               |
            //                                                 R | o   o   o   S | x
            //       --------------> X-axis                      |               |
            //                                                 R | o   o   o   S | x
            //                                                   +---------------+
            //                                                 R   x   x   x   S   x     R --- recv from nbr_WEST

            // SEND to east
            int sx = nn + (MX-1) - i;
            int sy = 0;
            int sz = 0;

            // RECV from west
            int rx = (nn - 1) - i;
            int ry = 0;
            int rz = 0;

            int send = sx + sy * MXP + sz * MXP*MYP; // send the bottommost (non-ghost) layer of data
            int recv = rx + ry * MXP + rz * MXP*MYP; // receive data into the top ghost cell layer

            MPI_Sendrecv(&PDF3d[send],       // send buffer (points to the starting address of the data chunk) 
                         1,                  // number of elements to be sent
                         stridex,            // type of elements
                         nbr_EAST,           // destination (where the data is going)
                         333,                // tag
                         &PDF3d[recv],       // receive buffer (points to the starting address of the data chunk)
                         1,                  // number of elements received
                         stridex,            // type of elements
                         nbr_WEST,           // source (where the data is coming from)
                         333,                // tag
                         CART_COMM,          // MPI Communicator used for this Sendrecv
                         &status);           // MPI status
        }

        // I am sending PDF3d data to the process nbr_WEST and receiving PDF3d data from process nbr_EAST
        {
            //                                                 x   S   x   x   x   R
            // send the westmost (non-ghost) layer of data       +---------------+
            // receive data into the east ghost cell layer     x | S   o   o   o | R     S --- send to nbr_WEST
            //                                                   |               |
            //                                                 x | S   o   o   o | R
            //       --------------> X-axis                      |               |
            //                                                 x | S   o   o   o | R
            //                                                   +---------------+
            //                                                 x   S   x   x   x   R     R --- recv from nbr_EAST

            // SEND to west
            int sx = nn + i;
            int sy = 0;
            int sz = 0;

            // RECV from east
            int rx = nn + MX + i;
            int ry = 0;
            int rz = 0;

            int send = sx + sy * MXP + sz * MXP*MYP; // send the bottommost (non-ghost) layer of data
            int recv = rx + ry * MXP + rz * MXP*MYP; // receive data into the top ghost cell layer

            MPI_Sendrecv(&PDF3d[send],       // send buffer (points to the starting address of the data chunk) 
                         1,                  // number of elements to be sent
                         stridex,            // type of elements
                         nbr_WEST,           // destination (where the data is going)
                         444,                // tag
                         &PDF3d[recv],       // receive buffer (points to the starting address of the data chunk)
                         1,                  // number of elements received
                         stridex,            // type of elements
                         nbr_EAST,           // source (where the data is coming from)
                         444,                // tag
                         CART_COMM,          // MPI Communicator used for this Sendrecv
                         &status);           // MPI status
        }

        // I am sending PDF3d data to the process nbr_NORTH and receiving PDF3d data from process nbr_SOUTH
        {
            //                                                 R   x   x   x   S   x
            // send the northmost (non-ghost) layer of data      +---------------+
            // receive data into the south ghost cell layer    R | o   o   o   S | x     S --- send to nbr_NORTH
            //                                                   |               |
            //                                                 R | o   o   o   S | x
            //       --------------> Y-axis                      |               |
            //                                                 R | o   o   o   S | x
            //                                                   +---------------+
            //                                                 R   x   x   x   S   x     R --- recv from nbr_SOUTH

            // SEND to north
            int sx = 0;
            int sy = nn + (MY-1) - i;
            int sz = 0;

            // RECV from south
            int rx = 0;
            int ry = (nn - 1) - i;
            int rz = 0;

            int send = sx + sy * MXP + sz * MXP*MYP; // send the bottommost (non-ghost) layer of data
            int recv = rx + ry * MXP + rz * MXP*MYP; // receive data into the top ghost cell layer

            MPI_Sendrecv(&PDF3d[send],       // send buffer (points to the starting address of the data chunk) 
                         1,                  // number of elements to be sent
                         stridey,            // type of elements
                         nbr_NORTH,          // destination (where the data is going)
                         555,                // tag
                         &PDF3d[recv],       // receive buffer (points to the starting address of the data chunk)
                         1,                  // number of elements received
                         stridey,            // type of elements
                         nbr_SOUTH,          // source (where the data is coming from)
                         555,                // tag
                         CART_COMM,          // MPI Communicator used for this Sendrecv
                         &status);           // MPI status
        }

        // I am sending PDF3d data to the process nbr_SOUTH and receiving PDF3d data from process nbr_NORTH
        {
            //                                                 x   S   x   x   x   R
            // send the southmost (non-ghost) layer of data      +---------------+
            // receive data into the north ghost cell layer    x | S   o   o   o | R     S --- send to nbr_SOUTH
            //                                                   |               |
            //                                                 x | S   o   o   o | R
            //       --------------> Y-axis                      |               |
            //                                                 x | S   o   o   o | R
            //                                                   +---------------+
            //                                                 x   S   x   x   x   R     R --- recv from nbr_NORTH

            // SEND to south
            int sx = 0;
            int sy = nn + i;
            int sz = 0;

            // RECV from north
            int rx = 0;
            int ry = nn + MY + i;
            int rz = 0;

            int send = sx + sy * MXP + sz * MXP*MYP; // send the bottommost (non-ghost) layer of data
            int recv = rx + ry * MXP + rz * MXP*MYP; // receive data into the top ghost cell layer

            MPI_Sendrecv(&PDF3d[send],       // send buffer (points to the starting address of the data chunk) 
                         1,                  // number of elements to be sent
                         stridey,            // type of elements
                         nbr_SOUTH,          // destination (where the data is going)
                         666,                // tag
                         &PDF3d[recv],       // receive buffer (points to the starting address of the data chunk)
                         1,                  // number of elements received
                         stridey,            // type of elements
                         nbr_NORTH,          // source (where the data is coming from)
                         666,                // tag
                         CART_COMM,          // MPI Communicator used for this Sendrecv
                         &status);           // MPI status
        }

    } // end for loop over the number of ghost layers


        // loop over all voxels in this MPI process, including ghost layers 
        for(int i = 0; i < MXP; i++) {
            for(int j = 0; j < MYP; j++) {
                for(int k = 0; k < MZP; k++) {

                    // natural index for fa(i,j,k) in PDF3d
                    int index_3d = i + j*MXP + k*MXP*MYP;

                    // natural index for f(i,j,k,a) in PDF4d
                    int index_4d = a + (index_3d * Q);

                    // PDF4d <---- PDF3d(a)
                    PDF4d[index_4d] = PDF3d[index_3d];
                }
            }
        }

        // cleanup
        MPI_Type_free(&stridex);
        MPI_Type_free(&stridey);

    } // end loop for PDF directions

    // free memory for the temporary 3D array
    delete [] PDF3d;
}
