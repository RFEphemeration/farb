#ifndef FARB_FONTS_HPP
#define FARB_FONTS_HPP

namespace Farb
{

namespace UI
{

struct FontNameTag
{
	static HString GetName() { return "FontName"; }
};

using FontName = NamedType<std::string, FontNameTag>;

} // namespace UI

} // namespace Farb

#endif // FARB_FONTS_HPP