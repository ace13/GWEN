#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
#include "Gwen/Renderers/SFML.h"
#include <SFML/Graphics.hpp>
#include <GL/gl.h>

namespace Gwen 
{
	namespace Renderer 
	{
        struct TextureData
        {
            TextureData(sf::Image* img): image(img), texture(NULL) { }
            TextureData(sf::Texture* text): texture(text), image(NULL) { }
            ~TextureData() { if (texture != NULL) delete texture; if (image != NULL) delete image; }

            sf::Texture* texture;
            sf::Image*   image;
        };


		SFML::SFML( sf::RenderTarget& target ) : m_Target(target)
		{
		}

		SFML::~SFML()
		{

		}

		void SFML::SetDrawColor( Gwen::Color color )
		{
			m_Color.r = color.r;
			m_Color.g = color.g;
			m_Color.b = color.b;
			m_Color.a = color.a;
		}

		void SFML::DrawFilledRect( Gwen::Rect rect )
		{
			Translate( rect );

#if SFML_VERSION_MAJOR == 2
			sf::RectangleShape rectShape( sf::Vector2f( rect.w, rect.h ) );
			rectShape.setPosition( rect.x, rect.y );
			rectShape.setFillColor( m_Color );

			m_Target.draw( rectShape );
#else
			m_Target.Draw( sf::Shape::Rectangle( rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, m_Color ) );
#endif
		}
        
        void SFML::DrawShavedCornerRect( Gwen::Rect rect, bool bSlight )
        {
#if SFML_VERSION_MAJOR == 2
            Translate( rect );

#define VERT(X, Y) sf::Vertex(sf::Vector2f(X,Y),m_Color)

            sf::Vertex verts[] = { VERT(rect.x + 1, rect.y), VERT(rect.x + rect.w - 1, rect.y),
                                   VERT(rect.x + rect.w, rect.y + 1), VERT(rect.x + rect.w, rect.y + rect.h - 1),
                                   VERT(rect.x + rect.w - 1, rect.y + rect.h), VERT(rect.x + 1, rect.y + rect.h),
                                   VERT(rect.x, rect.y + rect.h - 1), VERT(rect.x, rect.y + 1),
                                   VERT(rect.x + 1, rect.y) };

            if (!bSlight)
            {
                verts[0].position.x += 1;
                verts[1].position.x -= 1;
                verts[2].position.y += 1;
                verts[3].position.y -= 1;
                verts[4].position.x -= 1;
                verts[5].position.x += 1;
                verts[6].position.y -= 1;
                verts[7].position.y += 1;
                verts[8].position.x += 1;
            }

#undef VERT

            m_Target.draw( verts, 9, sf::PrimitiveType::LinesStrip );
#else
            Base::DrawShavedCornerRect( rect, bSlight );
#endif
        }

        void SFML::DrawLinedRect( Gwen::Rect rect )
        {
#if SFML_VERSION_MAJOR == 2
            Translate( rect );

            sf::RectangleShape rectShape( sf::Vector2f( rect.w-2, rect.h-2 ) );
            rectShape.setPosition( rect.x+1, rect.y+1 );
            rectShape.setFillColor( sf::Color::Transparent );
            rectShape.setOutlineColor( m_Color );
            rectShape.setOutlineThickness( 1.f );

            m_Target.draw( rectShape );
#else
            Base::DrawLinedRect( rect );
#endif
        }

        void SFML::StartImage( Gwen::Rect rect )
        {
            m_ImageRect = rect;
            Translate( m_ImageRect );

            m_Image.create(rect.w, rect.h);
        }

        void SFML::EndImage()
        {
            sf::Texture imageTex;
            imageTex.loadFromImage( m_Image );
            
            sf::RectangleShape rectShape( sf::Vector2f( m_ImageRect.w, m_ImageRect.h ) );
			rectShape.setPosition( m_ImageRect.x, m_ImageRect.y );
			rectShape.setTexture( &imageTex );

            m_Target.draw( rectShape );

            m_Image = sf::Image();
        }

        void SFML::DrawPixel( int x, int y )
        {
#if SFML_VERSION_MAJOR == 2
            if (m_Image.getSize() != sf::Vector2u())
            {
                m_Image.setPixel(x, y, m_Color);
            }
            else
            {
                Translate( x, y );

                sf::Vertex vert( sf::Vector2f( x, y ), m_Color );

                m_Target.draw( &vert, 1, sf::PrimitiveType::Points );
            }
#else
            Base::DrawPixel( x, y );
#endif
        }

		void SFML::LoadFont( Gwen::Font* font )
		{
			font->realsize = font->size * Scale();

			sf::Font* pFont = new sf::Font();

#if SFML_VERSION_MAJOR == 2
			if ( !pFont->loadFromFile( Utility::UnicodeToString( font->facename ) ) )
#else
			if ( !pFont->LoadFromFile( Utility::UnicodeToString( font->facename ), font->realsize  ) )
#endif
			{
				// Ideally here we should be setting the font to a system default font here.
				delete pFont;
#if SFML_VERSION_MAJOR == 2
                pFont = NULL; // SFML 2 doesn't have a default font anymore
#else
				static sf::Font defaultFont = sf::Font::GetDefaultFont();
				pFont = &defaultFont;
#endif
			}
			
			font->data = pFont;
		}

		void SFML::FreeFont( Gwen::Font* pFont )
		{
			if ( !pFont->data ) return;

			sf::Font* font = ((sf::Font*)pFont->data);

#if SFML_VERSION_MAJOR != 2
			// If this is the default font then don't delete it!
			if ( font != &sf::Font::GetDefaultFont() )
			{
				delete font;
			}
#endif

			pFont->data = NULL;
		}

		void SFML::RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text )
		{
			Translate( pos.x, pos.y );

			// If the font doesn't exist, or the font size should be changed
			if ( !pFont->data || fabs( pFont->realsize - pFont->size * Scale() ) > 2 )
			{
				FreeFont( pFont );
				LoadFont( pFont );
			}

			const sf::Font* pSFFont = (sf::Font*)(pFont->data);

#if SFML_VERSION_MAJOR != 2
			if  ( !pSFFont )
			{
				static sf::Font defaultFont = sf::Font::GetDefaultFont();
				pSFFont = &defaultFont;
			}
#endif

			#if SFML_VERSION_MAJOR == 2
				sf::Text sfStr;
                sfStr.setString( text );
				sfStr.setFont( *pSFFont );
				sfStr.move( pos.x, pos.y );
				sfStr.setCharacterSize( pFont->realsize );
				sfStr.setColor( m_Color );
				m_Target.draw( sfStr );
			#else
				sf::String sfStr( text );
				sfStr.SetFont( *pSFFont );
				sfStr.Move( pos.x, pos.y );
				sfStr.SetSize( pFont->realsize );
				sfStr.SetColor( m_Color );
				m_Target.Draw( sfStr );
			#endif

			
		}

		Gwen::Point SFML::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text )
		{
			// If the font doesn't exist, or the font size should be changed
			if ( !pFont->data || fabs( pFont->realsize - pFont->size * Scale() ) > 2 )
			{
				FreeFont( pFont );
				LoadFont( pFont );
			}

			const sf::Font* pSFFont = (sf::Font*)(pFont->data);

#if SFML_VERSION_MAJOR != 2
			if  ( !pSFFont )
			{
				static sf::Font defaultFont = sf::Font::GetDefaultFont();
				pSFFont = &defaultFont;
			}
#endif

			#if SFML_VERSION_MAJOR == 2
				sf::Text sfStr;
                sfStr.setString( text );
				sfStr.setFont( *pSFFont );
				sfStr.setCharacterSize( pFont->realsize );
				sf::FloatRect sz = sfStr.getLocalBounds();
				return Gwen::Point( sz.left + sz.width, sz.top + sz.height );
			#else
				sf::String sfStr( text );
				sfStr.SetFont( *pSFFont );
				sfStr.SetSize( pFont->realsize );
				sf::FloatRect sz = sfStr.GetRect();
				return Gwen::Point( sz.GetWidth(), sz.GetHeight() );
			#endif
			
		}

		void SFML::StartClip()
		{
			Gwen::Rect rect = ClipRegion();

			// OpenGL's coords are from the bottom left
			// so we need to translate them here.
			{
				GLint view[4];
				glGetIntegerv( GL_VIEWPORT, &view[0] );
				rect.y = view[3] - (rect.y + rect.h);
			}

			glScissor( rect.x * Scale(), rect.y * Scale(), rect.w * Scale(), rect.h * Scale() );
			glEnable( GL_SCISSOR_TEST );
		};


		void SFML::EndClip()
		{
			glDisable( GL_SCISSOR_TEST );
		};

		void SFML::LoadTexture( Gwen::Texture* pTexture )
		{
			if ( !pTexture ) return;
			if ( pTexture->data ) FreeTexture( pTexture );

#if SFML_VERSION_MAJOR == 2
			sf::Texture* tex = new sf::Texture();
            tex->setSmooth( true );
            if ( !tex->loadFromFile( pTexture->name.Get() ) )
#else
			sf::Image* tex = new sf::Image();
            tex->SetSmooth( true );
            if ( !tex->LoadFromFile( pTexture->name.Get() ) )
#endif
			{
				delete( tex );
				pTexture->failed = true;
				return;
			}

#if SFML_VERSION_MAJOR == 2
            pTexture->height = tex->getSize().x;
			pTexture->width = tex->getSize().y;
#else
			pTexture->height = tex->GetHeight();
			pTexture->width = tex->GetWidth();
#endif
			pTexture->data = new TextureData(tex);

		};

		void SFML::FreeTexture( Gwen::Texture* pTexture )
		{
            TextureData* data = static_cast<TextureData*>( pTexture->data );

			if ( data )
			{
				delete data;
			}

			pTexture->data = NULL;
		}

		void SFML::DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect rect, float u1, float v1, float u2, float v2 )
		{
            TextureData* data = static_cast<TextureData*>( pTexture->data );

            if ( !data )
                return DrawMissingImage( rect );

#if SFML_VERSION_MAJOR == 2
			const sf::Texture* tex = data->texture;
#else 
			const sf::Image* tex = data->image;
#endif 

			if ( !tex ) 
				return DrawMissingImage( rect );

			Translate( rect );
		
#if SFML_VERSION_MAJOR == 2
            sf::Vector2u texSize = tex->getSize();
            u1 *= texSize.x;
			v1 *= texSize.y;

			u2 *= texSize.x;
			u2 -= u1;

			v2 *= texSize.y;
			v2 -= v1;

			sf::RectangleShape rectShape( sf::Vector2f( rect.w, rect.h ) );
			rectShape.setPosition( rect.x, rect.y );
			rectShape.setTexture( tex );
			rectShape.setTextureRect( sf::IntRect( u1, v1, u2, v2 ) );
			
			m_Target.draw( rectShape );
#else
			tex->Bind();

			glColor4f(1, 1, 1, 1 );

			glBegin( GL_QUADS );
				glTexCoord2f( u1, v1 );		glVertex2f(rect.x,     rect.y);
				glTexCoord2f( u1, v2 );		glVertex2f(rect.x,     rect.y + rect.h);
				glTexCoord2f( u2, v2 );		glVertex2f(rect.x + rect.w, rect.y + rect.h);
				glTexCoord2f( u2, v1 );		glVertex2f(rect.x + rect.w, rect.y) ;
			glEnd();

			glBindTexture( GL_TEXTURE_2D, 0);
#endif
		}

		Gwen::Color SFML::PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default )
		{
            TextureData* data = static_cast<TextureData*>( pTexture->data );

			#if SFML_VERSION_MAJOR == 2

				if ( !data->texture && !data->image ) return col_default;
                if ( !data->image )
                {
                    sf::Image copy = data->texture->copyToImage();
                    data->image = new sf::Image(copy);
                }

				sf::Color col = data->image->getPixel( x, y );
				return Gwen::Color( col.r, col.g, col.b, col.a );

			#else 

				const sf::Image* tex = data->image;
				if ( !tex ) return col_default;

				sf::Color col = tex->GetPixel( x, y );
				return Gwen::Color( col.r, col.g, col.b, col.a );

			#endif 

		}

	
	}
}
