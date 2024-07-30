///////////////////////////////////////////////////////////////////////////////
//
//  TestYaml.cpp
//
//  Copyright © Pete Isensee (PKIsensee@msn.com).
//  All rights reserved worldwide.
//
//  Permission to copy, modify, reproduce or redistribute this source code is
//  granted provided the above copyright notice is retained in the resulting 
//  source code.
// 
//  This software is provided "as is" and without any express or implied
//  warranties.
//
///////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>

#include "yaml.h"
#include "File.h"

// Macros
#ifdef _DEBUG
#define verify(e) assert(e)
#define test(e)   assert(e)
#else
#define verify(e) static_cast<void>(e)
#define test(e)   static_cast<void>( (e) || ( Util::DebugBreak(), 0 ) )
#endif

using namespace PKIsensee;
namespace fs = std::filesystem;

///////////////////////////////////////////////////////////////////////////////

struct TestYamlHandler : public YamlHandler
{
  bool isEarlyOut = false;
  bool errorHappened = false;
  void onStartDocument() override
  {
    std::cout << "onStartDocument\n";
  }

  void onEndDocument() override
  {
    std::cout << "onEndDocument\n";
  }

  void onStartSequence() override
  {
    std::cout << "onStartSequence\n";
  }

  void onEndSequence() override
  {
    std::cout << "onEndSequence\n";
  }

  void onStartMapping() override
  {
    std::cout << "onStartMapping\n";
  }

  void onEndMapping() override
  {
    std::cout << "onEndMapping\n";
  }

  bool onKey( std::string_view key ) override
  {
    std::cout << "key: " << key << "\n";
    return true;
  }

  bool onScalar( std::string_view scalar ) override
  {
    std::cout << "scalar: " << scalar << "\n";
    if( scalar == "QuitQuitQuit" )
    {
      isEarlyOut = true;
      return false;
    }
    return true;
  }

  void onError( std::string_view err, size_t line, size_t col ) override
  {
    errorHappened = true;
    std::cout << "ERROR: " << err << " on line " << line << " col " << col << "\n";
  }
};

const std::string kYamlText =
"# Song data\r\n"
"Song:\r\n"
" Rating: 5\r\n"
" ArtistGender : F\r\n"
" Moods :\r\n"
"  - Happy # This makes me happy\r\n"
"  - Mellow\r\n"
"  - Dinner\r\n"
" Seasons :\r\n"
" Holidays: # No holidays specified \r\n"
" LeadInstruments:\r\n"
"  - Vocal\r\n"
" Movies :\r\n"
" AltGenres:\r\n"
"  - Pop\r\n"
"  - R&B\r\n"
" AltArtists : [Moby, John Williams]  \r\n"
" AltAlbums :\r\n"
" AltTitles :\r\n"
" AltYears : \r\n"
" AltComposers :\r\n"
" - Linda Thompson\r\n"
" Languages :\r\n"
" Games:";

void TestMultiFile()
{
  for( const auto& entry : fs::directory_iterator( ".\\TestFiles" ) )
  {
    fs::path path = entry.path();
    if( path.extension() == ".yaml" )
    {
      std::string yaml;
      File::ReadEntireFile( path, yaml );

      TestYamlHandler testYamlHandler;
      YamlParser parser( yaml, testYamlHandler );
      auto result = parser.Parse();
      if( !testYamlHandler.isEarlyOut )
        test( result );
    }
  }
}

int __cdecl main( int, char** )
{
  // Valid YAML
  TestYamlHandler testYamlHandler;
  YamlParser parser( kYamlText, testYamlHandler );
  auto result = parser.Parse();
  test( result );

  // Tabs
  YamlParser parseTabs( "\r\n\t ", testYamlHandler );
  result = parseTabs.Parse();
  test( !result );
  test( testYamlHandler.errorHappened );

  // Unterminated string
  testYamlHandler.errorHappened = false;
  YamlParser parseUntermStr( "\"unterminated string", testYamlHandler );
  result = parseUntermStr.Parse();
  test( !result );
  test( testYamlHandler.errorHappened );

  TestMultiFile();

  // YAML creation
  test( Yaml::CreateKeyValue( "key", "" ) == "key: \n" );
  test( Yaml::CreateKeyValue( "key", "value" ) == "key: value\n");
  test( Yaml::CreateKeyValue( "key", "\"value\"" ) == "key: \"value\"\n" );
  test( Yaml::CreateKeyValue( "key", "#" ) == "key: '#'\n" );
  test( Yaml::CreateKeyValue( "key", "va\'lue" ) == "key: \"va\'lue\"\n" );
  test( Yaml::CreateKeyValue( "key", "va\"lue" ) == "key: \'va\"lue\'\n" );
  //test( Yaml::CreateKeyValue( "key", "va\'lu\"e" ) == "key: \"va\'lue\"\n" ); // assertion

  std::vector<std::string> seq;
  test( Yaml::CreateSequence( seq) == "[]");
  seq.push_back( "first" );
  test( Yaml::CreateSequence( seq) == "[first]");
  seq.push_back( "second" );
  test( Yaml::CreateSequence( seq) == "[first, second]");

  std::vector<int> iseq;
  test( Yaml::CreateKeyValueSeq( "key", iseq ) == "key: []\n" );
  iseq.push_back( 0 );
  test( Yaml::CreateKeyValueSeq( "key", iseq ) == "key: [0]\n" );
  iseq.push_back( 1 );
  test( Yaml::CreateKeyValueSeq( "key", iseq ) == "key: [0, 1]\n" );

  return 0;
}
