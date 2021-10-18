/*
 * parse input ini file (default "resources.conf", change with -f <name> )
 * create output (default "resources.h", change with -o <name> ) file with
 * resource classes
 */
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "INIReader.h"
#include "base64.h"

using namespace std;

int main(int argc, char **argv) {
  string ini = "resources.conf";
  string out = "resources.h";
  string dir = ".";

  for (int i = 1; i < argc; i++)
    if (!strcmp(argv[i], "-f")) {
      ini = string(argv[++i]);
    } else if (!strcmp(argv[i], "-o")) {
      out = string(argv[++i]);
    } else if (!strcmp(argv[i], "-d")) {
      dir = string(argv[++i]);
    }

  INIReader reader(ini);
  if (reader.ParseError() < 0) {
    cerr << "cannot parse config file: " << ini << endl;
    exit(1);
  }

  fstream f(out, fstream::out);
  f << "#ifndef ___RESOURCES___" << endl;
  f << "#define ___RESOURCES___" << endl << endl;
  f << "#include <string>" << endl << endl;
  f << "#include \"base64.h\"" << endl << endl;

  for (auto section : reader.GetSections()) {
    f << "namespace " << section << " {" << endl;
    unordered_map<string, string> fields;
    for (auto field : reader.GetFields(section)) {
      stringstream ss;
      ss << dir << "/" << reader.Get(section, field, "");
      FILE *rf;
      char *buffer;
      long numbytes;
      if (!(rf = fopen(ss.str().data(), "r"))) continue;
      fseek(rf, 0L, SEEK_END);
      numbytes = ftell(rf);
      fseek(rf, 0L, SEEK_SET);
      buffer = (char *)calloc(numbytes, sizeof(char));
      fread(buffer, sizeof(char), numbytes, rf);
      fclose(rf);
      fields[field] = Base64::Encode(buffer, numbytes);
      free(buffer);
    }
    for (auto field : fields) {
      f << "  std::string " << field.first << "() {  " << endl;
      f << "      std::string ret; " << endl;
      f << "      Base64::Decode(\"" << field.second << "\","
        << field.second.size() << ", ret);" << endl;
      f << "      return ret; " << endl;
      f << "  } " << endl;
    }
    f << "};" << endl << endl;
  }
  f << "#endif" << endl;
  f.close();
}
