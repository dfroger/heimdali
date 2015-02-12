#include <tclap/CmdLine.h>

#include <itkImage.h>
#include <itkExpImageFilter.h>
#include <itkVectorImageToImageAdaptor.h>
#include <itkComposeImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkINRImageIOFactory.h>

#include "heimdali/cmdreader.hxx"
#include "heimdali/cmdwriter.hxx"
#include "heimdali/itkhelper.hxx"

using namespace std;

int main(int argc, char** argv)
{ 

try {

TCLAP::CmdLine cmd("Compute exponential of each pixel", ' ', "0.0.0");

// -o output.h5
TCLAP::ValueArg<string> output("o","image_out", 
    "Output image, instead of standard output",false,"","FILENAME", cmd);

// input.h5
TCLAP::UnlabeledValueArg<string> input0("image_in", 
    "First image.",true,"","IMAGE-IN",cmd);

cmd.parse(argc,argv);

// Put our INRimage reader in the list of readers ITK knows.
itk::ObjectFactoryBase::RegisterFactory( itk::INRImageIOFactory::New() ); 

// Image type.
typedef itk::Image<Heimdali::PixelFloat, Heimdali::ImageDimension> ScalarImageType;
typedef itk::VectorImage<Heimdali::PixelFloat, Heimdali::ImageDimension> VectorImageType;
VectorImageType::Pointer vectorImage;

// Command line tool readers.
typedef Heimdali::CmdReader<VectorImageType> ReaderType;
ReaderType* cmdreader = ReaderType::make_cmd_reader(0, input0.getValue());

// Command line tool writer.
typedef Heimdali::CmdWriter<VectorImageType> WriterType;
WriterType* cmdwriter = WriterType::make_cmd_writer(output.getValue());

// Duplicator.
typedef itk::ImageDuplicator<ScalarImageType> DuplicatorType;
DuplicatorType::Pointer duplicator = DuplicatorType::New();

// VectorImage to Image
typedef itk::VectorImageToImageAdaptor
    <Heimdali::PixelFloat, Heimdali::ImageDimension> ToImageType;
ToImageType::Pointer toImage = ToImageType::New();

// Exp filter.
typedef itk::ExpImageFilter <ToImageType,ScalarImageType> ExpType;
ExpType::Pointer expfilter = ExpType::New();

// Image to VectorImage
typedef itk::ComposeImageFilter<ScalarImageType> ToVectorImageType;
ToVectorImageType::Pointer toVectorImage = ToVectorImageType::New();

size_t iregionmax = 1E+06;
for (size_t iregion=0 ; iregion<iregionmax ; iregion++) {
    // Read input.
    cmdreader->next_iteration();
    if (cmdreader->is_complete()) break;

    cmdreader->reader()->Update();

    vectorImage = cmdreader->GetOutput();

    toImage->SetImage(vectorImage);

    for (int ic = 0 ; ic < cmdreader->get_sc() ; ++ic)
    {
        // VectorImage to Image
        toImage->SetExtractComponentIndex(ic);
        toImage->Modified();

        // Exp images.
        expfilter->SetInput(toImage);
        expfilter->Update();
        expfilter->Modified();

        // Image to VectorImage
        duplicator->SetInputImage(expfilter->GetOutput());
        duplicator->Update();

        // Image to VectorImage
        toVectorImage->SetInput(ic, duplicator->GetOutput());
        toVectorImage->Modified();
        toVectorImage->Update();
    }

    // Write output.
    cmdwriter->Write( toVectorImage->GetOutput() );
    cmdwriter->Update();

    vectorImage->Initialize();
}

} // End of 'try' block.


// Command line parser.
catch (TCLAP::ArgException &e) { 
    cerr << "par: ERROR: " << e.error() << " for arg " << e.argId() << endl;
}


// Input/output.
catch (Heimdali::IOError &e) {
    cerr << "par: ERROR: " << e.getMessage() << endl;
}

catch (Heimdali::NotImplementedError &e) {
    cerr << "par: ERROR: " << e.getMessage() << endl;
}

return 0;

}
