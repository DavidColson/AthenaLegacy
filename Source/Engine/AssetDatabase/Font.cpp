#include "Font.h"

#include "Rendering/RectPacking.h"

void Font::Load(FileSys::FilePath path)
{   
    FT_Face face;

    FT_Error err = FT_New_Face(FontSystem::GetFreeType(), path.fullPath().c_str(), 0, &face);
	if (err)
	{
		Log::Warn("FreeType Error: %s", FT_Error_String(err));
	}
    
    // Rasterize the entire font to a texture atlas
	// **********************************************

	// Texture data
	int texHeight = 512;
	int texWidth = 512;
	uint8_t* pTextureDataAsR8{ nullptr };
	pTextureDataAsR8 = new uint8_t[texHeight * texWidth];
	memset(pTextureDataAsR8, 0, texHeight * texWidth);

	eastl::vector<Packing::Rect> rects;
	rects.get_allocator().set_name("Font Packing Rects");
	characters.get_allocator().set_name("Font character data");

	FT_Set_Pixel_Sizes(face, 0, 50);

	// Prepare rects for packing
	for (int i = 0; i < 128; i++)
	{
		FT_Load_Char(face, i, FT_LOAD_DEFAULT);
		Packing::Rect newRect;
		newRect.w = face->glyph->bitmap.width + 1;
		newRect.h = face->glyph->bitmap.rows + 1;
		rects.push_back(newRect);
	}
	Packing::SkylinePackRects(rects, texWidth, texHeight);

	for (int i = 0; i < 128; i++)
	{
		Packing::Rect& rect = rects[i];
		FT_Load_Char(face, i, FT_LOAD_RENDER);

		// Create the character with all it's appropriate data
		Character character;
		character.size = Vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows);
		character.bearing = Vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top);
		character.advance = (face->glyph->advance.x) >> 6;
		character.UV0 = Vec2f((float)rect.x / (float)texWidth, (float)rect.y / (float)texHeight);
		character.UV1 = Vec2f((float)(rect.x + character.size.x) / (float)texWidth, (float)(rect.y + character.size.y) / (float)texHeight);
		characters.push_back(character);

		// Blit the glyph's image into our texture atlas
		uint8_t* pSourceData = face->glyph->bitmap.buffer;
		uint8_t* pDestination = pTextureDataAsR8 + rect.y * texWidth + rect.x;
		int sourceDataPitch = face->glyph->bitmap.pitch;
		for (uint32_t y = 0; y < face->glyph->bitmap.rows; y++, pSourceData += sourceDataPitch, pDestination += texWidth)
		{
			memcpy(pDestination, pSourceData, face->glyph->bitmap.width);
		}
	}

	fontTexture = GfxDevice::CreateTexture(texWidth, texHeight, TextureFormat::R8, pTextureDataAsR8, "Font Atlas");
	delete[] pTextureDataAsR8;
}

Font::~Font()
{
    GfxDevice::FreeTexture(fontTexture);
}