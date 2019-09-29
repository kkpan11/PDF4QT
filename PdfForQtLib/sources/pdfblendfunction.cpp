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

#include "pdfblendfunction.h"

namespace pdf
{

constexpr const std::pair<const char*, BlendMode> BLEND_MODE_INFOS[] =
{
    { "Normal", BlendMode::Normal },
    { "Multiply", BlendMode::Multiply },
    { "Screen", BlendMode::Screen },
    { "Overlay", BlendMode::Overlay },
    { "Darken", BlendMode::Darken },
    { "Lighten", BlendMode::Lighten },
    { "ColorDodge", BlendMode::ColorDodge },
    { "ColorBurn", BlendMode::ColorBurn },
    { "HardLight", BlendMode::HardLight },
    { "SoftLight", BlendMode::SoftLight },
    { "Difference", BlendMode::Difference },
    { "Exclusion", BlendMode::Exclusion },
    { "Hue", BlendMode::Hue },
    { "Saturation", BlendMode::Saturation },
    { "Color", BlendMode::Color },
    { "Luminosity", BlendMode::Luminosity },
    { "Compatible", BlendMode::Compatible }
};

BlendMode PDFBlendModeInfo::getBlendMode(const QByteArray& name)
{
    for (const std::pair<const char*, BlendMode>& info : BLEND_MODE_INFOS)
    {
        if (info.first == name)
        {
            return info.second;
        }
    }

    return BlendMode::Invalid;
}

bool PDFBlendModeInfo::isSupportedByQt(BlendMode mode)
{
    switch (mode)
    {
       case BlendMode::Normal:
       case BlendMode::Multiply:
       case BlendMode::Screen:
       case BlendMode::Overlay:
       case BlendMode::Darken:
       case BlendMode::Lighten:
       case BlendMode::ColorDodge:
       case BlendMode::ColorBurn:
       case BlendMode::HardLight:
       case BlendMode::SoftLight:
       case BlendMode::Difference:
       case BlendMode::Exclusion:
       case BlendMode::Compatible:
            return true;

        default:
            return false;
    }
}

QPainter::CompositionMode PDFBlendModeInfo::getCompositionModeFromBlendMode(BlendMode mode)
{
    switch (mode)
    {
        case BlendMode::Normal:
            return QPainter::CompositionMode_SourceOver;
        case BlendMode::Multiply:
            return QPainter::CompositionMode_Multiply;
        case BlendMode::Screen:
            return QPainter::CompositionMode_Screen;
        case BlendMode::Overlay:
            return QPainter::CompositionMode_Overlay;
        case BlendMode::Darken:
            return QPainter::CompositionMode_Darken;
        case BlendMode::Lighten:
            return QPainter::CompositionMode_Lighten;
        case BlendMode::ColorDodge:
            return QPainter::CompositionMode_ColorDodge;
        case BlendMode::ColorBurn:
            return QPainter::CompositionMode_ColorBurn;
        case BlendMode::HardLight:
            return QPainter::CompositionMode_HardLight;
        case BlendMode::SoftLight:
            return QPainter::CompositionMode_SoftLight;
        case BlendMode::Difference:
            return QPainter::CompositionMode_Difference;
        case BlendMode::Exclusion:
            return QPainter::CompositionMode_Exclusion;
        case BlendMode::Compatible:
            return QPainter::CompositionMode_SourceOver;

        default:
            break;
    }

    return QPainter::CompositionMode_SourceOver;
}

QString PDFBlendModeInfo::getBlendModeName(BlendMode mode)
{
    for (const std::pair<const char*, BlendMode>& info : BLEND_MODE_INFOS)
    {
        if (info.second == mode)
        {
            return QString::fromLatin1(info.first);
        }
    }

    return "Unknown";
}

}   // namespace pdf