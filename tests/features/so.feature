Feature: so

    Scenario: Invoke so
        When I run the command: so --help
        Then I see the line in standard output: Subtract two images

    Scenario: Subtract two images
        When I run the command: so -o so_tmp.h5 $HEIMDALI_DATA_DIR/imtest_z5_y4_x3_c2.h5 $HEIMDALI_DATA_DIR/imtest_z5_y4_x3_c2.h5
        Then images so_tmp.h5 and $HEIMDALI_DATA_DIR/imtest_z5_y4_x3_c2_zero.h5 are equal
