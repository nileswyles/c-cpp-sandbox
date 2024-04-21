#include <string>
using namespace std;

// Assuming unix paths for now... 
// TODO: extend using classes? to support Windows?
namespace SueroLibs::Paths {

extern string getParent(string path);
extern string getFilename(string path);
extern string getExtension(string path);

}