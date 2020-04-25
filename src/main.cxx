#include <cstdlib>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include "WindowManager.hpp"
#include "app.h"

using namespace std;
namespace po = boost::program_options;

po::options_description getDescription()
{
  po::options_description desc("Options");
  desc.add_options()("help,h", "Display this message")("version,v", "Display the version number");

  return desc;
}

int main(int argc, char *argv[])
{
  po::options_description desc = getDescription();
  po::variables_map varmap;

  try
  {
    store(po::command_line_parser(argc, argv).options(desc).run(), varmap);
    notify(varmap);
  }
  catch (po::error &error)
  {
    cerr << error.what() << endl;
    return EXIT_FAILURE;
  }

  if (varmap.count("help"))
  {
    cout << desc << endl;
  }

  if (varmap.count("version"))
  {
    App app;
    cout << app.getProjectVersion() << endl;
  }

  BOOST_LOG_TRIVIAL(info) << "Initializing the window manager.";
  unique_ptr<WindowManager> window_manager = WindowManager::Create();
  if (!window_manager)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to initialize window manager.";
    return EXIT_FAILURE;
  }

  window_manager->Run();

  return EXIT_SUCCESS;
}
