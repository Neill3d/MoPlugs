
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Panorama.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "Panorama.h"
#include <curl/curl.h>
#include <rapidjson\rapidjson.h>
#include <rapidjson\document.h>
#include <vector>

#include "SOIL.h"

#include "IO\FileUtils.h"

#define USER_AGENT "StreetView MoBu Plugin 1.0"

double defLocation[2] = {51.50700703827454, -0.12791916931155356};

const int levelsW[] = { 1, 2, 4, 7, 13, 26 };
const int levelsH[] = { 1, 1, 2, 4, 7, 13 };

const int widths[] = { 416, 832, 1664, 3328, 6656, 13312 };
const int heights[] = { 416, 416, 832, 1664, 3328, 6656 };

const float maxW = 1024.0f;
const float maxH = 1024.0f;

/////////////////////////////////////////////////////////////////////////////////////////////////////

size_t write_download_data(void *ptr, size_t size, size_t nmemb, std::vector<unsigned char>* b) {
    size_t oldSize = b->size();
    try {
        b->resize(b->size() + size * nmemb + 1);
    } catch (const std::bad_alloc&) {
        throw "download failed: out of memory";
    }
    memcpy(&(*b)[oldSize], ptr, size * nmemb);
    return size * nmemb;
}


/**
 * Download the contents of a url as a block of memory
 *
 * @param url
 * @return
 */
std::auto_ptr<std::vector<unsigned char> > download(const char *url) {
    //setStatus("Downloading %s\n", url);

    std::auto_ptr<std::vector<unsigned char> > b(new std::vector<unsigned char>());

	FBString effectPath, effectFullName;

	if (FindEffectLocation("ca-bundle.crt", effectPath, effectFullName) )
	{
		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_CAINFO, effectFullName);
		//curl_easy_setopt(curl, CURLOPT_RETURNTRANSFER, 1);
		//curl_easy_setopt(curl, CURLOPT_BINARYTRANSFER,1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_download_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, b.get());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);


		CURLcode res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);


		//Throw a human readable string back to the caller if something went wrong
		if (res != 0)
			throw curl_easy_strerror(res);

		b.get()->data() [ b.get()->size() ] = '\0';
	}
	else
	{
		FBMessageBox( "Panorama", "Failed to find certificate file", "Ok" );
	}

	return b;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void downloadImage(const char *filename, const char *url) {
    //setStatus("Downloading %s\n", url);

	FILE *fp = 0;
	fopen_s(&fp, filename, "wb");

	FBString effectPath, effectFullName;

	if (FindEffectLocation("ca-bundle.crt", effectPath, effectFullName) )
	{

		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_CAINFO, effectFullName);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);
		//curl_easy_setopt(curl, CURLOPT_RETURNTRANSFER, 1);
		//curl_easy_setopt(curl, CURLOPT_BINARYTRANSFER,1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);

	

		CURLcode res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);


		//Throw a human readable string back to the caller if something went wrong
		if (res != 0)
			throw curl_easy_strerror(res);
	}
	else
	{
		FBMessageBox( "Panorama", "Failed to find certificate file", "Ok" );
	}
	
    if (fp>0) fclose(fp);

}

CPanorama::CPanorama()
{
	mPanoCanvas = nullptr;
	mZoom = 4;
	AdaptTextureToZoom();
}

CPanorama::~CPanorama()
{
	Free();
}

void CPanorama::Free()
{
	if (mPanoCanvas)
	{
		delete [] mPanoCanvas;
		mPanoCanvas = nullptr;
	}
}

bool CPanorama::Load(const char *folder, const double lat, const double lng, const int zoom)
{
	Free();

	mZoom = zoom;
	if (mZoom < 1) mZoom = 1;
	if (mZoom > 5) mZoom = 5;
	AdaptTextureToZoom();

	double location[2];

	location[0] = (lat == 0.0) ? defLocation[0] : lat;
	location[1] = (lng == 0.0) ? defLocation[1] : lng;


	char buffer[1024];

	//memset(baseurl, 0, sizeof(char)*1024);
	memset(buffer, 0, sizeof(char) * 1024);

	sprintf_s(buffer, 1024, "%s%lf,%lf&radius=350", "https://cbks0.google.com/cbk?cb_client=maps_sv.tactile&authuser=0&hl=en&output=polygon&it=1%3A1&rank=closest&ll=", location[0], location[1] );

	const std::auto_ptr<std::vector<unsigned char> > data = download(buffer);
	
	if (data.get()->size() == 0)
		return false;
	
	rapidjson::Document		document;

	if (document.Parse<0>( (char*)data.get()->data() ).HasParseError() )
		return false;
	/*
	var data = JSON.parse( http_request.responseText );
	//self.loadPano( location, data.Location.panoId );
	self.loadPano( location, data.result[ 0 ].id );
	*/

	if (document.HasMember("result") )
	{
		rapidjson::Value &a = document["result"];
		rapidjson::Type type = a.GetType();
		if ( a.IsArray() )
		{
			type = a[0u].GetType();
			if ( a[0u].HasMember("id") )
			{
				rapidjson::Value::Member *id = a[0u].FindMember("id");

				size_t stringLength = id->value.GetStringLength();
				std::string str;
				str.resize(stringLength);
				str = id->value.GetString();

				mId = str;
				LoadPano(folder, location[0], location[1], str.c_str() );
			}
		}
		
	}

	return true;
}


bool CPanorama::LoadPano(const char *folder, const double lat, const double lng, const char *id)
{
	ComposePanorama(folder);
	return true;
}

bool FileExists(LPCTSTR szPath)
{
  DWORD dwAttrib = GetFileAttributes(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool CPanorama::ComposePanorama(const char *folder)
{
	int w = levelsW[mZoom];
	int h = levelsH[mZoom];

	char name[MAX_PATH];
	char buffer[1024];
	memset(name, 0, sizeof(char) *64);
	memset(buffer, 0, sizeof(char) * 1024);

	for (int y=0; y<h; ++y)
		for (int x=0; x<w; ++x)
		{
			sprintf_s(buffer, 1024, "%s%s%s%d%s%d%s%d%s", "https://geo0.ggpht.com/cbk?cb_client=maps_sv.tactile&authuser=0&hl=en&panoid=", mId.c_str(), "&output=tile&x=", x, "&y=", y, "&zoom=", mZoom, "&nbt&fover=2" );

			// load texture tile
			//const std::auto_ptr<std::vector<unsigned char> > data = download(buffer);

			sprintf_s(name, MAX_PATH, "%s\\%d.jpg", folder, x*1000 + y);

			//if ( FileExists(name) )
			//{
				downloadImage(name, buffer);
			//}

			//SaveTile(name, data.get()->size(), data.get()->data() );
			ComposeFromTile( x, y, name );
			
		}

	// save resulted panorama to dist
	sprintf_s(name, MAX_PATH, "%s\\panorama.tga", folder );
	SOIL_save_image( name, SOIL_SAVE_TYPE_TGA, mPanoWidth, mPanoHeight, 3, mPanoCanvas );

	return true;
}

void CPanorama::AdaptTextureToZoom()
{
	int w = widths[mZoom];
	int h = heights[mZoom];

	mPanoWidth = w;
	mPanoHeight = h;

	mPanoCanvas = new unsigned char[w*h*3];

	mWc = ceil( 1.0f * w / maxW );
	mHc = ceil( 1.0f * h / maxH );


	for (int y=0; y< mHc; ++y)
		for (int x=0; x<mWc; ++x)
		{
			int cWidth = (x<(mWc-1)) ? maxW : w - (maxW * x );
			int cHeight = (y<(mHc-1)) ? maxH : h - (maxH * y);
		}
}

bool CPanorama::ComposeFromTile(const int tile_x, const int tile_y, const char *filename )
{

	int lx = tile_x * 512;
	int ly = tile_y * 512;
	float px = floor( 1.0f * lx / maxW );
	float py = floor( 1.0f * ly / maxH );

	const int width = widths[mZoom];

	lx -= px * maxW;
	ly -= py * maxH;

	int imgWidth, imgHeight, imgChannels;

	unsigned char *tile_data = SOIL_load_image(filename, &imgWidth, &imgHeight, &imgChannels, 3);
	
	if (tile_data == nullptr)
		return false;

	//Copy each pixel from the tile to the global texture
	for (unsigned int y = 0; y < imgHeight; y++) {
        for (unsigned int x = 0; x < imgWidth; x++) {

            const int global_x = tile_x * imgWidth + x;
            const int global_y = tile_y * imgHeight + y;

			if (global_x >= mPanoWidth || global_y >= mPanoHeight)
                continue;

			mPanoCanvas[(global_y * width + global_x)*3 + 0] = tile_data[(y * imgWidth + x)*3 + 0];
			mPanoCanvas[(global_y * width + global_x)*3 + 1] = tile_data[(y * imgWidth + x)*3 + 1];
            mPanoCanvas[(global_y * width + global_x)*3 + 2] = tile_data[(y * imgWidth + x)*3 + 2];
        }
    }

	SOIL_free_image_data(tile_data);

	return true;
}

bool CPanorama::SaveTile(const char *name, const int length, const unsigned char *data)
{

	FILE *fp = 0;
	fopen_s(&fp, name, "wb");

	fwrite(data, sizeof(unsigned char), length, fp );

	fclose(fp);

	return true;
}