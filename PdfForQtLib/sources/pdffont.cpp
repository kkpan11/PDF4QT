//    Copyright (C) 2019 Jakub Melka
//
//    This file is part of PdfForQt.
//
//    PdfForQt is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    PdfForQt is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDFForQt.  If not, see <https://www.gnu.org/licenses/>.

#include "pdffont.h"
#include "pdfdocument.h"
#include "pdfparser.h"
#include "pdfnametounicode.h"

namespace pdf
{

PDFFont::PDFFont(FontDescriptor fontDescriptor) :
    m_fontDescriptor(qMove(fontDescriptor))
{

}

PDFFontPointer PDFFont::createFont(const PDFObject& object, const PDFDocument* document)
{
    const PDFObject& dereferencedFontDictionary = document->getObject(object);
    if (!dereferencedFontDictionary.isDictionary())
    {
        throw PDFParserException(PDFTranslationContext::tr("Font object must be a dictionary."));
    }

    const PDFDictionary* fontDictionary = dereferencedFontDictionary.getDictionary();
    PDFDocumentDataLoaderDecorator fontLoader(document);

    // TODO: Fonts - implement all types of the font
    // First, determine the font subtype
    constexpr const std::array<std::pair<const char*, FontType>, 2> fontTypes = {
        std::pair<const char*, FontType>{ "Type1", FontType::Type1 },
        std::pair<const char*, FontType>{ "TrueType", FontType::TrueType }
    };

    const FontType fontType = fontLoader.readEnumByName(fontDictionary->get("Subtype"), fontTypes.cbegin(), fontTypes.cend(), FontType::Invalid);
    if (fontType == FontType::Invalid)
    {
        throw PDFParserException(PDFTranslationContext::tr("Invalid font type."));
    }

    QByteArray name = fontLoader.readNameFromDictionary(fontDictionary, "Name");
    QByteArray baseFont = fontLoader.readNameFromDictionary(fontDictionary, "BaseFont");
    const PDFInteger firstChar = fontLoader.readIntegerFromDictionary(fontDictionary, "FirstChar", 0);
    const PDFInteger lastChar = fontLoader.readIntegerFromDictionary(fontDictionary, "LastChar", 255);
    std::vector<PDFInteger> widths = fontLoader.readIntegerArrayFromDictionary(fontDictionary, "Widths");

    // Read standard font
    constexpr const std::array<std::pair<const char*, StandardFontType>, 14> standardFonts = {
        std::pair<const char*, StandardFontType>{ "Times-Roman", StandardFontType::TimesRoman },
        std::pair<const char*, StandardFontType>{ "Times-Bold", StandardFontType::TimesRomanBold },
        std::pair<const char*, StandardFontType>{ "Times-Italic", StandardFontType::TimesRomanItalics },
        std::pair<const char*, StandardFontType>{ "Times-BoldItalic", StandardFontType::TimesRomanBoldItalics },
        std::pair<const char*, StandardFontType>{ "Helvetica", StandardFontType::Helvetica },
        std::pair<const char*, StandardFontType>{ "Helvetica-Bold", StandardFontType::HelveticaBold },
        std::pair<const char*, StandardFontType>{ "Helvetica-Oblique", StandardFontType::HelveticaOblique },
        std::pair<const char*, StandardFontType>{ "Helvetica-BoldOblique", StandardFontType::HelveticaBoldOblique },
        std::pair<const char*, StandardFontType>{ "Courier", StandardFontType::Courier },
        std::pair<const char*, StandardFontType>{ "Courier-Bold", StandardFontType::CourierBold },
        std::pair<const char*, StandardFontType>{ "Courier-Oblique", StandardFontType::CourierOblique },
        std::pair<const char*, StandardFontType>{ "Courier-BoldOblique", StandardFontType::CourierBoldOblique },
        std::pair<const char*, StandardFontType>{ "Symbol", StandardFontType::Symbol },
        std::pair<const char*, StandardFontType>{ "ZapfDingbats", StandardFontType::ZapfDingbats }
    };
    const StandardFontType standardFont = fontLoader.readEnumByName(fontDictionary->get("BaseFont"), standardFonts.cbegin(), standardFonts.cend(), StandardFontType::Invalid);

    // Read Font Descriptor
    FontDescriptor fontDescriptor;
    const PDFObject& fontDescriptorObject = document->getObject(fontDictionary->get("FontDescriptor"));
    if (fontDescriptorObject.isDictionary())
    {
        const PDFDictionary* fontDescriptorDictionary = fontDescriptorObject.getDictionary();
        fontDescriptor.fontName = fontLoader.readNameFromDictionary(fontDescriptorDictionary, "FontName");
        fontDescriptor.fontFamily = fontLoader.readStringFromDictionary(fontDescriptorDictionary, "FontFamily");

        constexpr const std::array<std::pair<const char*, QFont::Stretch>, 9> stretches = {
            std::pair<const char*, QFont::Stretch>{ "UltraCondensed", QFont::UltraCondensed },
            std::pair<const char*, QFont::Stretch>{ "ExtraCondensed", QFont::ExtraCondensed },
            std::pair<const char*, QFont::Stretch>{ "Condensed", QFont::Condensed },
            std::pair<const char*, QFont::Stretch>{ "SemiCondensed", QFont::SemiCondensed },
            std::pair<const char*, QFont::Stretch>{ "Normal", QFont::Unstretched },
            std::pair<const char*, QFont::Stretch>{ "SemiExpanded", QFont::SemiExpanded },
            std::pair<const char*, QFont::Stretch>{ "Expanded", QFont::Expanded },
            std::pair<const char*, QFont::Stretch>{ "ExtraExpanded", QFont::ExtraExpanded },
            std::pair<const char*, QFont::Stretch>{ "UltraExpanded", QFont::UltraExpanded }
        };
        fontDescriptor.fontStretch = fontLoader.readEnumByName(fontDescriptorDictionary->get("FontStretch"), stretches.cbegin(), stretches.cend(), QFont::Unstretched);
        fontDescriptor.fontWeight = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "FontWeight", 500);
        fontDescriptor.italicAngle = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "ItalicAngle", 0.0);
        fontDescriptor.ascent = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "Ascent", 0.0);
        fontDescriptor.descent = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "Descent", 0.0);
        fontDescriptor.leading = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "Leading", 0.0);
        fontDescriptor.capHeight = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "CapHeight", 0.0);
        fontDescriptor.xHeight = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "XHeight", 0.0);
        fontDescriptor.stemV = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "StemV", 0.0);
        fontDescriptor.stemH = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "StemH", 0.0);
        fontDescriptor.avgWidth = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "AvgWidth", 0.0);
        fontDescriptor.maxWidth = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "MaxWidth", 0.0);
        fontDescriptor.missingWidth = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "MissingWidth", 0.0);
        fontDescriptor.flags = fontLoader.readIntegerFromDictionary(fontDescriptorDictionary, "Flags", 0);
        fontDescriptor.boundingBox = fontLoader.readRectangle(fontDescriptorDictionary->get("FontBBox"), QRectF());
        fontDescriptor.charset = fontLoader.readStringFromDictionary(fontDescriptorDictionary, "Charset");

        auto loadStream = [fontDescriptorDictionary, document](QByteArray& byteArray, const char* name)
        {
            if (fontDescriptorDictionary->hasKey(name))
            {
                const PDFObject& streamObject = document->getObject(fontDescriptorDictionary->get(name));
                if (streamObject.isStream())
                {
                    byteArray = document->getDecodedStream(streamObject.getStream());
                }
            }
        };
        loadStream(fontDescriptor.fontFile, "FontFile");
        loadStream(fontDescriptor.fontFile2, "FontFile2");
        loadStream(fontDescriptor.fontFile3, "FontFile3");
    }

    // Read Font Encoding
    // The font encoding for the simple font is determined by this algorithm:
    //      1) Try to use Encoding dictionary to determine base encoding
    //         (it can be MacRomanEncoding, MacExpertEncoding, WinAnsiEncoding or StandardEncoding)
    //      2) If it is not present, then try to obtain built-in encoding from the font file (usually, this is not possible)
    //      3) Use default encoding for the font depending on the font type
    //          - one of the 14 base fonts - use builtin encoding for the font type
    //          - TrueType - use WinAnsiEncoding
    //          - all others - use StandardEncoding
    //      4) Merge with Differences, if present
    //      5) Fill missing characters from StandardEncoding

    // TODO: Read font encoding from the font file
    PDFEncoding::Encoding encoding = PDFEncoding::Encoding::Invalid;
    encoding::EncodingTable simpleFontEncodingTable = { };
    switch (fontType)
    {
        case FontType::Type1:
        case FontType::TrueType:
        {
            bool hasDifferences = false;
            encoding::EncodingTable differences = { };

            if (fontDictionary->hasKey("Encoding"))
            {
                constexpr const std::array<std::pair<const char*, PDFEncoding::Encoding>, 3> encodings = {
                    std::pair<const char*, PDFEncoding::Encoding>{ "MacRomanEncoding", PDFEncoding::Encoding::MacRoman },
                    std::pair<const char*, PDFEncoding::Encoding>{ "MacExpertEncoding", PDFEncoding::Encoding::MacExpert },
                    std::pair<const char*, PDFEncoding::Encoding>{ "WinAnsiEncoding", PDFEncoding::Encoding::WinAnsi }
                };

                const PDFObject& encodingObject = document->getObject(fontDictionary->get("Encoding"));
                if (encodingObject.isName())
                {
                    // Decode name of the encoding
                    encoding = fontLoader.readEnumByName(encodingObject, encodings.cbegin(), encodings.cend(), PDFEncoding::Encoding::Invalid);
                }
                else if (encodingObject.isDictionary())
                {
                    // Dictionary with base encoding and differences (all optional)
                    const PDFDictionary* encodingDictionary = encodingObject.getDictionary();
                    if (encodingDictionary->hasKey("BaseEncoding"))
                    {
                        encoding = fontLoader.readEnumByName(encodingDictionary->get("BaseEncoding"), encodings.cbegin(), encodings.cend(), PDFEncoding::Encoding::Invalid);
                    }
                    else
                    {
                        // We get encoding for the standard font. If we have invalid standard font,
                        // then we get standard encoding. So we shouldn't test it.
                        encoding = getEncodingForStandardFont(standardFont);
                    }

                    if (encodingDictionary->hasKey("Differences"))
                    {
                        const PDFObject& differencesArray = document->getObject(encodingDictionary->get("Differences"));
                        if (differencesArray.isArray())
                        {
                            hasDifferences = true;
                            const PDFArray* array = differencesArray.getArray();
                            size_t currentOffset = 0;
                            for (size_t i = 0, count = array->getCount(); i < count; ++i)
                            {
                                const PDFObject& item = document->getObject(array->getItem(i));
                                if (item.isInt())
                                {
                                    currentOffset = static_cast<size_t>(item.getInteger());
                                }
                                else if (item.isName())
                                {
                                    if (currentOffset >= differences.size())
                                    {
                                        throw PDFParserException(PDFTranslationContext::tr("Invalid differences in encoding entry of the font."));
                                    }

                                    QChar character = PDFNameToUnicode::getUnicodeForName(item.getString());

                                    // Try ZapfDingbats, if this fails
                                    if (character.isNull())
                                    {
                                        character = PDFNameToUnicode::getUnicodeForNameZapfDingbats(item.getString());
                                    }
                                    differences[currentOffset] = character;

                                    ++currentOffset;
                                }
                                else
                                {
                                    throw PDFParserException(PDFTranslationContext::tr("Invalid differences in encoding entry of the font."));
                                }
                            }
                        }
                        else
                        {
                            throw PDFParserException(PDFTranslationContext::tr("Invalid differences in encoding entry of the font."));
                        }
                    }
                }
                else
                {
                    throw PDFParserException(PDFTranslationContext::tr("Invalid encoding entry of the font."));
                }
            }
            else
            {
                // We get encoding for the standard font. If we have invalid standard font,
                // then we get standard encoding. So we shouldn't test it.
                encoding = getEncodingForStandardFont(standardFont);
            }

            if (encoding == PDFEncoding::Encoding::Invalid)
            {
                throw PDFParserException(PDFTranslationContext::tr("Invalid encoding entry of the font."));
            }

            simpleFontEncodingTable = *PDFEncoding::getTableForEncoding(encoding);

            // Fill in differences
            if (hasDifferences)
            {
                for (size_t i = 0; i < differences.size(); ++i)
                {
                    if (!differences[i].isNull())
                    {
                        simpleFontEncodingTable[i] = differences[i];
                    }
                }

                // Set the encoding to custom
                encoding = PDFEncoding::Encoding::Custom;
            }

            // Fill in missing characters from standard encoding
            const encoding::EncodingTable& standardEncoding = *PDFEncoding::getTableForEncoding(PDFEncoding::Encoding::Standard);
            for (size_t i = 0; i < standardEncoding.size(); ++i)
            {
                if ((simpleFontEncodingTable[i].isNull() || simpleFontEncodingTable[i] == QChar(QChar::SpecialCharacter::ReplacementCharacter)) &&
                    (!standardEncoding[i].isNull() && standardEncoding[i] != QChar(QChar::SpecialCharacter::ReplacementCharacter)))
                {
                    simpleFontEncodingTable[i] = standardEncoding[i];
                }
            }

            break;
        }

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    switch (fontType)
    {
        case FontType::Type1:
            return PDFFontPointer(new PDFType1Font(qMove(fontDescriptor), qMove(name), qMove(baseFont), firstChar, lastChar, qMove(widths), encoding, simpleFontEncodingTable, standardFont));

        case FontType::TrueType:
            return PDFFontPointer(new PDFTrueTypeFont(qMove(fontDescriptor), qMove(name), qMove(baseFont), firstChar, lastChar, qMove(widths), encoding, simpleFontEncodingTable));

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    // Read To Unicode
    // TODO: Read To Unicode

    // Read Embedded fonts
    // TODO: Read embedded fonts
    return PDFFontPointer();
}

PDFSimpleFont::PDFSimpleFont(FontDescriptor fontDescriptor,
                             QByteArray name,
                             QByteArray baseFont,
                             PDFInteger firstChar,
                             PDFInteger lastChar,
                             std::vector<PDFInteger> widths,
                             PDFEncoding::Encoding encodingType,
                             encoding::EncodingTable encoding) :
    PDFFont(qMove(fontDescriptor)),
    m_name(qMove(name)),
    m_baseFont(qMove(baseFont)),
    m_firstChar(firstChar),
    m_lastChar(lastChar),
    m_widths(qMove(widths)),
    m_encodingType(encodingType),
    m_encoding(encoding)
{

}

QRawFont PDFSimpleFont::getRealizedFont(PDFReal fontSize) const
{
    // TODO: Fix font creation to use also embedded fonts, font descriptor, etc.
    QRawFont rawFont;

    if (m_fontDescriptor.isEmbedded())
    {
        // Type 1 font
        if (!m_fontDescriptor.fontFile.isEmpty())
        {
            rawFont.loadFromData(m_fontDescriptor.fontFile, fontSize, QFont::PreferNoHinting);
        }
        else if (!m_fontDescriptor.fontFile2.isEmpty())
        {
            rawFont.loadFromData(m_fontDescriptor.fontFile2, fontSize, QFont::PreferNoHinting);
        }

        if (!rawFont.isValid())
        {
            throw PDFParserException(PDFTranslationContext::tr("Can't load embedded font."));
        }
    }
    else
    {
        // TODO: Zkontrolovat, zda se zde opravdu prebiraji spravne fonty
        const int weight = qBound<int>(0, m_fontDescriptor.fontWeight / 10.0, 99);
        const int stretch = qBound<int>(1, m_fontDescriptor.fontStretch, 4000);

        QFont font(m_baseFont);
        font.setHintingPreference(QFont::PreferNoHinting);
        font.setStretch(stretch);
        font.setWeight(weight);
        font.setPixelSize(fontSize);
        rawFont = QRawFont::fromFont(font, QFontDatabase::Any);
    }

    return rawFont;
}

QString PDFSimpleFont::getTextUsingEncoding(const QByteArray& byteArray) const
{
    QString string;
    string.resize(byteArray.size(), QChar());

    for (int i = 0, count = byteArray.size(); i < count; ++i)
    {
        string[i] = m_encoding[static_cast<uint8_t>(byteArray[i])];
    }

    return string;
}

PDFType1Font::PDFType1Font(FontDescriptor fontDescriptor,
                           QByteArray name,
                           QByteArray baseFont,
                           PDFInteger firstChar,
                           PDFInteger lastChar,
                           std::vector<PDFInteger> widths,
                           PDFEncoding::Encoding encodingType,
                           encoding::EncodingTable encoding,
                           StandardFontType standardFontType) :
    PDFSimpleFont(qMove(fontDescriptor), qMove(name), qMove(baseFont), firstChar, lastChar, qMove(widths), encodingType, encoding),
    m_standardFontType(standardFontType)
{

}

FontType PDFType1Font::getFontType() const
{
    return FontType::Type1;
}

FontType PDFTrueTypeFont::getFontType() const
{
    return FontType::TrueType;
}

}   // namespace pdf
