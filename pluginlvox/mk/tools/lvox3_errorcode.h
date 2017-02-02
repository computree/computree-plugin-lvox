/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_ERRORCODE_H
#define LVOX3_ERRORCODE_H


namespace lvox {

    /**
     * @brief Error or warning code that must be used in grid
     */
    enum ErrorOrWarningCode {
        B_Equals_C = -1,                    // error
        B_Inferior_C = -2,                  // error
        B_Minus_C_Inferior_Threshold = -3,  // warning
        A_Superior_B_Minus_C = -4,          // error
        A_Equals_B_Minus_C = -5,            // warning
        IDR_Surestimated = -6,              // warning
        Max_Error_Code = -7,                // don't use it ! it is just to set NA() in grid

        // Same as previous but with Nb, Nt, Ni
        Nt_Equals_Nb = B_Equals_C,
        Nt_Inferior_Nb = B_Inferior_C,
        Nt_Minus_Nb_Inferior_Threshold = B_Minus_C_Inferior_Threshold,
        Ni_Superior_Nt_Minus_Nb = A_Superior_B_Minus_C,
        Ni_Equals_Nt_Minus_Nb = A_Equals_B_Minus_C,

        // Same as previous but more explicit
        All_theoretical_rays_have_already_been_intercepted = B_Equals_C,
        A_voxel_has_more_theoretical_rays_than_returned = B_Inferior_C,
        Too_little_information_for_IDR_calculation = B_Minus_C_Inferior_Threshold,
        A_voxel_has_returned_more_rays_than_incoming_rays = A_Superior_B_Minus_C,
        All_incoming_rays_were_returned = A_Equals_B_Minus_C,
        Probable_probability_of_number_of_returned_rays = IDR_Surestimated,

        // other
        Zero_Division = B_Equals_C
    };
}

#endif // LVOX3_ERRORCODE_H
