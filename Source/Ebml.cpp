#include "Ebml.h"
#include "Endianness.h"

#include <inttypes.h>
#include <algorithm>

static EbmlElementDescriptor ebml_element_descriptors[] =
{
    { EbmlElementId::EBML, EbmlElementType::Master, 0, "EBML" },
    { EbmlElementId::EBMLVersion, EbmlElementType::UnsignedInteger, 1, "EBMLVersion" },
    { EbmlElementId::EBMLReadVersion, EbmlElementType::UnsignedInteger, 1, "EBMLReadVersion" },
    { EbmlElementId::EBMLMaxIDLength, EbmlElementType::UnsignedInteger, 1, "EBMLMaxIDLength" },
    { EbmlElementId::EBMLMaxSizeLength, EbmlElementType::UnsignedInteger, 1, "EBMLMaxSizeLength" },
    { EbmlElementId::DocType, EbmlElementType::String, 1, "DocType" },
    { EbmlElementId::DocTypeVersion, EbmlElementType::UnsignedInteger, 1, "DocTypeVersion" },
    { EbmlElementId::DocTypeReadVersion, EbmlElementType::UnsignedInteger, 1, "DocTypeReadVersion" },
    { EbmlElementId::Void, EbmlElementType::Binary, -1, "Void" },
    { EbmlElementId::CRC32, EbmlElementType::Binary, -1, "CRC-32" },
    { EbmlElementId::SignatureSlot, EbmlElementType::Master, -1, "SignatureSlot" },
    { EbmlElementId::SignatureAlgo, EbmlElementType::UnsignedInteger, 1, "SignatureAlgo" },
    { EbmlElementId::SignatureHash, EbmlElementType::UnsignedInteger, 1, "SignatureHash" },
    { EbmlElementId::SignaturePublicKey, EbmlElementType::Binary, 1, "SignaturePublicKey" },
    { EbmlElementId::Signature, EbmlElementType::Binary, 1, "Signature" },
    { EbmlElementId::SignatureElements, EbmlElementType::Master, 1, "SignatureElements" },
    { EbmlElementId::SignatureElementList, EbmlElementType::Master, 2, "SignatureElementList" },
    { EbmlElementId::SignedElement, EbmlElementType::Binary, 3, "SignedElement" },
    { EbmlElementId::Segment, EbmlElementType::Master, 0, "Segment" },
    { EbmlElementId::SeekHead, EbmlElementType::Master, 1, "SeekHead" },
    { EbmlElementId::Seek, EbmlElementType::Master, 2, "Seek" },
    { EbmlElementId::SeekID, EbmlElementType::Binary, 3, "SeekID" },
    { EbmlElementId::SeekPosition, EbmlElementType::UnsignedInteger, 3, "SeekPosition" },
    { EbmlElementId::Info, EbmlElementType::Master, 1, "Info" },
    { EbmlElementId::SegmentUID, EbmlElementType::Binary, 2, "SegmentUID" },
    { EbmlElementId::SegmentFilename, EbmlElementType::Utf8String, 2, "SegmentFilename" },
    { EbmlElementId::PrevUID, EbmlElementType::Binary, 2, "PrevUID" },
    { EbmlElementId::PrevFilename, EbmlElementType::Utf8String, 2, "PrevFilename" },
    { EbmlElementId::NextUID, EbmlElementType::Binary, 2, "NextUID" },
    { EbmlElementId::NextFilename, EbmlElementType::Utf8String, 2, "NextFilename" },
    { EbmlElementId::SegmentFamily, EbmlElementType::Binary, 2, "SegmentFamily" },
    { EbmlElementId::ChapterTranslate, EbmlElementType::Master, 2, "ChapterTranslate" },
    { EbmlElementId::ChapterTranslateEditionUID, EbmlElementType::UnsignedInteger, 3, "ChapterTranslateEditionUID" },
    { EbmlElementId::ChapterTranslateCodec, EbmlElementType::UnsignedInteger, 3, "ChapterTranslateCodec" },
    { EbmlElementId::ChapterTranslateID, EbmlElementType::Binary, 3, "ChapterTranslateID" },
    { EbmlElementId::TimecodeScale, EbmlElementType::UnsignedInteger, 2, "TimecodeScale" },
    { EbmlElementId::Duration, EbmlElementType::Float, 2, "Duration" },
    { EbmlElementId::DateUTC, EbmlElementType::Date, 2, "DateUTC" },
    { EbmlElementId::Title, EbmlElementType::Utf8String, 2, "Title" },
    { EbmlElementId::MuxingApp, EbmlElementType::Utf8String, 2, "MuxingApp" },
    { EbmlElementId::WritingApp, EbmlElementType::Utf8String, 2, "WritingApp" },
    { EbmlElementId::Cluster, EbmlElementType::Master, 1, "Cluster" },
    { EbmlElementId::Timecode, EbmlElementType::UnsignedInteger, 2, "Timecode" },
    { EbmlElementId::SilentTracks, EbmlElementType::Master, 2, "SilentTracks" },
    { EbmlElementId::SilentTrackNumber, EbmlElementType::UnsignedInteger, 3, "SilentTrackNumber" },
    { EbmlElementId::Position, EbmlElementType::UnsignedInteger, 2, "Position" },
    { EbmlElementId::PrevSize, EbmlElementType::UnsignedInteger, 2, "PrevSize" },
    { EbmlElementId::SimpleBlock, EbmlElementType::Binary, 2, "SimpleBlock" },
    { EbmlElementId::BlockGroup, EbmlElementType::Master, 2, "BlockGroup" },
    { EbmlElementId::Block, EbmlElementType::Binary, 3, "Block" },
    { EbmlElementId::BlockVirtual, EbmlElementType::Binary, 3, "BlockVirtual" },
    { EbmlElementId::BlockAdditions, EbmlElementType::Master, 3, "BlockAdditions" },
    { EbmlElementId::BlockMore, EbmlElementType::Master, 4, "BlockMore" },
    { EbmlElementId::BlockAddID, EbmlElementType::UnsignedInteger, 5, "BlockAddID" },
    { EbmlElementId::BlockAdditional, EbmlElementType::Binary, 5, "BlockAdditional" },
    { EbmlElementId::BlockDuration, EbmlElementType::UnsignedInteger, 3, "BlockDuration" },
    { EbmlElementId::ReferencePriority, EbmlElementType::UnsignedInteger, 3, "ReferencePriority" },
    { EbmlElementId::ReferenceBlock, EbmlElementType::SignedInteger, 3, "ReferenceBlock" },
    { EbmlElementId::ReferenceVirtual, EbmlElementType::SignedInteger, 3, "ReferenceVirtual" },
    { EbmlElementId::CodecState, EbmlElementType::Binary, 3, "CodecState" },
    { EbmlElementId::DiscardPadding, EbmlElementType::SignedInteger, 3, "DiscardPadding" },
    { EbmlElementId::Slices, EbmlElementType::Master, 3, "Slices" },
    { EbmlElementId::TimeSlice, EbmlElementType::Master, 4, "TimeSlice" },
    { EbmlElementId::LaceNumber, EbmlElementType::UnsignedInteger, 5, "LaceNumber" },
    { EbmlElementId::FrameNumber, EbmlElementType::UnsignedInteger, 5, "FrameNumber" },
    { EbmlElementId::BlockAdditionID, EbmlElementType::UnsignedInteger, 5, "BlockAdditionID" },
    { EbmlElementId::Delay, EbmlElementType::UnsignedInteger, 5, "Delay" },
    { EbmlElementId::SliceDuration, EbmlElementType::UnsignedInteger, 5, "SliceDuration" },
    { EbmlElementId::ReferenceFrame, EbmlElementType::Master, 3, "ReferenceFrame" },
    { EbmlElementId::ReferenceOffset, EbmlElementType::UnsignedInteger, 4, "ReferenceOffset" },
    { EbmlElementId::ReferenceTimeCode, EbmlElementType::UnsignedInteger, 4, "ReferenceTimeCode" },
    { EbmlElementId::EncryptedBlock, EbmlElementType::Binary, 2, "EncryptedBlock" },
    { EbmlElementId::Tracks, EbmlElementType::Master, 1, "Tracks" },
    { EbmlElementId::TrackEntry, EbmlElementType::Master, 2, "TrackEntry" },
    { EbmlElementId::TrackNumber, EbmlElementType::UnsignedInteger, 3, "TrackNumber" },
    { EbmlElementId::TrackUID, EbmlElementType::UnsignedInteger, 3, "TrackUID" },
    { EbmlElementId::TrackType, EbmlElementType::UnsignedInteger, 3, "TrackType" },
    { EbmlElementId::FlagEnabled, EbmlElementType::UnsignedInteger, 3, "FlagEnabled" },
    { EbmlElementId::FlagDefault, EbmlElementType::UnsignedInteger, 3, "FlagDefault" },
    { EbmlElementId::FlagForced, EbmlElementType::UnsignedInteger, 3, "FlagForced" },
    { EbmlElementId::FlagLacing, EbmlElementType::UnsignedInteger, 3, "FlagLacing" },
    { EbmlElementId::MinCache, EbmlElementType::UnsignedInteger, 3, "MinCache" },
    { EbmlElementId::MaxCache, EbmlElementType::UnsignedInteger, 3, "MaxCache" },
    { EbmlElementId::DefaultDuration, EbmlElementType::UnsignedInteger, 3, "DefaultDuration" },
    { EbmlElementId::DefaultDecodedFieldDuration, EbmlElementType::UnsignedInteger, 3, "DefaultDecodedFieldDuration" },
    { EbmlElementId::TrackTimecodeScale, EbmlElementType::Float, 3, "TrackTimecodeScale" },
    { EbmlElementId::TrackOffset, EbmlElementType::SignedInteger, 3, "TrackOffset" },
    { EbmlElementId::MaxBlockAdditionID, EbmlElementType::UnsignedInteger, 3, "MaxBlockAdditionID" },
    { EbmlElementId::Name, EbmlElementType::Utf8String, 3, "Name" },
    { EbmlElementId::Language, EbmlElementType::String, 3, "Language" },
    { EbmlElementId::CodecID, EbmlElementType::String, 3, "CodecID" },
    { EbmlElementId::CodecPrivate, EbmlElementType::Binary, 3, "CodecPrivate" },
    { EbmlElementId::CodecName, EbmlElementType::Utf8String, 3, "CodecName" },
    { EbmlElementId::AttachmentLink, EbmlElementType::UnsignedInteger, 3, "AttachmentLink" },
    { EbmlElementId::CodecSettings, EbmlElementType::Utf8String, 3, "CodecSettings" },
    { EbmlElementId::CodecInfoURL, EbmlElementType::String, 3, "CodecInfoURL" },
    { EbmlElementId::CodecDownloadURL, EbmlElementType::String, 3, "CodecDownloadURL" },
    { EbmlElementId::CodecDecodeAll, EbmlElementType::UnsignedInteger, 3, "CodecDecodeAll" },
    { EbmlElementId::TrackOverlay, EbmlElementType::UnsignedInteger, 3, "TrackOverlay" },
    { EbmlElementId::CodecDelay, EbmlElementType::UnsignedInteger, 3, "CodecDelay" },
    { EbmlElementId::SeekPreRoll, EbmlElementType::UnsignedInteger, 3, "SeekPreRoll" },
    { EbmlElementId::TrackTranslate, EbmlElementType::Master, 3, "TrackTranslate" },
    { EbmlElementId::TrackTranslateEditionUID, EbmlElementType::UnsignedInteger, 4, "TrackTranslateEditionUID" },
    { EbmlElementId::TrackTranslateCodec, EbmlElementType::UnsignedInteger, 4, "TrackTranslateCodec" },
    { EbmlElementId::TrackTranslateTrackID, EbmlElementType::Binary, 4, "TrackTranslateTrackID" },
    { EbmlElementId::Video, EbmlElementType::Master, 3, "Video" },
    { EbmlElementId::FlagInterlaced, EbmlElementType::UnsignedInteger, 4, "FlagInterlaced" },
    { EbmlElementId::FieldOrder, EbmlElementType::UnsignedInteger, 4, "FieldOrder" },
    { EbmlElementId::StereoMode, EbmlElementType::UnsignedInteger, 4, "StereoMode" },
    { EbmlElementId::AlphaMode, EbmlElementType::UnsignedInteger, 4, "AlphaMode" },
    { EbmlElementId::OldStereoMode, EbmlElementType::UnsignedInteger, 4, "OldStereoMode" },
    { EbmlElementId::PixelWidth, EbmlElementType::UnsignedInteger, 4, "PixelWidth" },
    { EbmlElementId::PixelHeight, EbmlElementType::UnsignedInteger, 4, "PixelHeight" },
    { EbmlElementId::PixelCropBottom, EbmlElementType::UnsignedInteger, 4, "PixelCropBottom" },
    { EbmlElementId::PixelCropTop, EbmlElementType::UnsignedInteger, 4, "PixelCropTop" },
    { EbmlElementId::PixelCropLeft, EbmlElementType::UnsignedInteger, 4, "PixelCropLeft" },
    { EbmlElementId::PixelCropRight, EbmlElementType::UnsignedInteger, 4, "PixelCropRight" },
    { EbmlElementId::DisplayWidth, EbmlElementType::UnsignedInteger, 4, "DisplayWidth" },
    { EbmlElementId::DisplayHeight, EbmlElementType::UnsignedInteger, 4, "DisplayHeight" },
    { EbmlElementId::DisplayUnit, EbmlElementType::UnsignedInteger, 4, "DisplayUnit" },
    { EbmlElementId::AspectRatioType, EbmlElementType::UnsignedInteger, 4, "AspectRatioType" },
    { EbmlElementId::ColourSpace, EbmlElementType::Binary, 4, "ColourSpace" },
    { EbmlElementId::GammaValue, EbmlElementType::Float, 4, "GammaValue" },
    { EbmlElementId::FrameRate, EbmlElementType::Float, 4, "FrameRate" },
    { EbmlElementId::Colour, EbmlElementType::Master, 4, "Colour" },
    { EbmlElementId::MatrixCoefficients, EbmlElementType::UnsignedInteger, 5, "MatrixCoefficients" },
    { EbmlElementId::BitsPerChannel, EbmlElementType::UnsignedInteger, 5, "BitsPerChannel" },
    { EbmlElementId::ChromaSubsamplingHorz, EbmlElementType::UnsignedInteger, 5, "ChromaSubsamplingHorz" },
    { EbmlElementId::ChromaSubsamplingVert, EbmlElementType::UnsignedInteger, 5, "ChromaSubsamplingVert" },
    { EbmlElementId::CbSubsamplingHorz, EbmlElementType::UnsignedInteger, 5, "CbSubsamplingHorz" },
    { EbmlElementId::CbSubsamplingVert, EbmlElementType::UnsignedInteger, 5, "CbSubsamplingVert" },
    { EbmlElementId::ChromaSitingHorz, EbmlElementType::UnsignedInteger, 5, "ChromaSitingHorz" },
    { EbmlElementId::ChromaSitingVert, EbmlElementType::UnsignedInteger, 5, "ChromaSitingVert" },
    { EbmlElementId::Range, EbmlElementType::UnsignedInteger, 5, "Range" },
    { EbmlElementId::TransferCharacteristics, EbmlElementType::UnsignedInteger, 5, "TransferCharacteristics" },
    { EbmlElementId::Primaries, EbmlElementType::UnsignedInteger, 5, "Primaries" },
    { EbmlElementId::MaxCLL, EbmlElementType::UnsignedInteger, 5, "MaxCLL" },
    { EbmlElementId::MaxFALL, EbmlElementType::UnsignedInteger, 5, "MaxFALL" },
    { EbmlElementId::MasteringMetadata, EbmlElementType::Master, 5, "MasteringMetadata" },
    { EbmlElementId::PrimaryRChromaticityX, EbmlElementType::Float, 6, "PrimaryRChromaticityX" },
    { EbmlElementId::PrimaryRChromaticityY, EbmlElementType::Float, 6, "PrimaryRChromaticityY" },
    { EbmlElementId::PrimaryGChromaticityX, EbmlElementType::Float, 6, "PrimaryGChromaticityX" },
    { EbmlElementId::PrimaryGChromaticityY, EbmlElementType::Float, 6, "PrimaryGChromaticityY" },
    { EbmlElementId::PrimaryBChromaticityX, EbmlElementType::Float, 6, "PrimaryBChromaticityX" },
    { EbmlElementId::PrimaryBChromaticityY, EbmlElementType::Float, 6, "PrimaryBChromaticityY" },
    { EbmlElementId::WhitePointChromaticityX, EbmlElementType::Float, 6, "WhitePointChromaticityX" },
    { EbmlElementId::WhitePointChromaticityY, EbmlElementType::Float, 6, "WhitePointChromaticityY" },
    { EbmlElementId::LuminanceMax, EbmlElementType::Float, 6, "LuminanceMax" },
    { EbmlElementId::LuminanceMin, EbmlElementType::Float, 6, "LuminanceMin" },
    { EbmlElementId::Audio, EbmlElementType::Master, 3, "Audio" },
    { EbmlElementId::SamplingFrequency, EbmlElementType::Float, 4, "SamplingFrequency" },
    { EbmlElementId::OutputSamplingFrequency, EbmlElementType::Float, 4, "OutputSamplingFrequency" },
    { EbmlElementId::Channels, EbmlElementType::UnsignedInteger, 4, "Channels" },
    { EbmlElementId::ChannelPositions, EbmlElementType::Binary, 4, "ChannelPositions" },
    { EbmlElementId::BitDepth, EbmlElementType::UnsignedInteger, 4, "BitDepth" },
    { EbmlElementId::TrackOperation, EbmlElementType::Master, 3, "TrackOperation" },
    { EbmlElementId::TrackCombinePlanes, EbmlElementType::Master, 4, "TrackCombinePlanes" },
    { EbmlElementId::TrackPlane, EbmlElementType::Master, 5, "TrackPlane" },
    { EbmlElementId::TrackPlaneUID, EbmlElementType::UnsignedInteger, 6, "TrackPlaneUID" },
    { EbmlElementId::TrackPlaneType, EbmlElementType::UnsignedInteger, 6, "TrackPlaneType" },
    { EbmlElementId::TrackJoinBlocks, EbmlElementType::Master, 4, "TrackJoinBlocks" },
    { EbmlElementId::TrackJoinUID, EbmlElementType::UnsignedInteger, 5, "TrackJoinUID" },
    { EbmlElementId::TrickTrackUID, EbmlElementType::UnsignedInteger, 3, "TrickTrackUID" },
    { EbmlElementId::TrickTrackSegmentUID, EbmlElementType::Binary, 3, "TrickTrackSegmentUID" },
    { EbmlElementId::TrickTrackFlag, EbmlElementType::UnsignedInteger, 3, "TrickTrackFlag" },
    { EbmlElementId::TrickMasterTrackUID, EbmlElementType::UnsignedInteger, 3, "TrickMasterTrackUID" },
    { EbmlElementId::TrickMasterTrackSegmentUID, EbmlElementType::Binary, 3, "TrickMasterTrackSegmentUID" },
    { EbmlElementId::ContentEncodings, EbmlElementType::Master, 3, "ContentEncodings" },
    { EbmlElementId::ContentEncoding, EbmlElementType::Master, 4, "ContentEncoding" },
    { EbmlElementId::ContentEncodingOrder, EbmlElementType::UnsignedInteger, 5, "ContentEncodingOrder" },
    { EbmlElementId::ContentEncodingScope, EbmlElementType::UnsignedInteger, 5, "ContentEncodingScope" },
    { EbmlElementId::ContentEncodingType, EbmlElementType::UnsignedInteger, 5, "ContentEncodingType" },
    { EbmlElementId::ContentCompression, EbmlElementType::Master, 5, "ContentCompression" },
    { EbmlElementId::ContentCompAlgo, EbmlElementType::UnsignedInteger, 6, "ContentCompAlgo" },
    { EbmlElementId::ContentCompSettings, EbmlElementType::Binary, 6, "ContentCompSettings" },
    { EbmlElementId::ContentEncryption, EbmlElementType::Master, 5, "ContentEncryption" },
    { EbmlElementId::ContentEncAlgo, EbmlElementType::UnsignedInteger, 6, "ContentEncAlgo" },
    { EbmlElementId::ContentEncKeyID, EbmlElementType::Binary, 6, "ContentEncKeyID" },
    { EbmlElementId::ContentSignature, EbmlElementType::Binary, 6, "ContentSignature" },
    { EbmlElementId::ContentSigKeyID, EbmlElementType::Binary, 6, "ContentSigKeyID" },
    { EbmlElementId::ContentSigAlgo, EbmlElementType::UnsignedInteger, 6, "ContentSigAlgo" },
    { EbmlElementId::ContentSigHashAlgo, EbmlElementType::UnsignedInteger, 6, "ContentSigHashAlgo" },
    { EbmlElementId::Cues, EbmlElementType::Master, 1, "Cues" },
    { EbmlElementId::CuePoint, EbmlElementType::Master, 2, "CuePoint" },
    { EbmlElementId::CueTime, EbmlElementType::UnsignedInteger, 3, "CueTime" },
    { EbmlElementId::CueTrackPositions, EbmlElementType::Master, 3, "CueTrackPositions" },
    { EbmlElementId::CueTrack, EbmlElementType::UnsignedInteger, 4, "CueTrack" },
    { EbmlElementId::CueClusterPosition, EbmlElementType::UnsignedInteger, 4, "CueClusterPosition" },
    { EbmlElementId::CueRelativePosition, EbmlElementType::UnsignedInteger, 4, "CueRelativePosition" },
    { EbmlElementId::CueDuration, EbmlElementType::UnsignedInteger, 4, "CueDuration" },
    { EbmlElementId::CueBlockNumber, EbmlElementType::UnsignedInteger, 4, "CueBlockNumber" },
    { EbmlElementId::CueCodecState, EbmlElementType::UnsignedInteger, 4, "CueCodecState" },
    { EbmlElementId::CueReference, EbmlElementType::Master, 4, "CueReference" },
    { EbmlElementId::CueRefTime, EbmlElementType::UnsignedInteger, 5, "CueRefTime" },
    { EbmlElementId::CueRefCluster, EbmlElementType::UnsignedInteger, 5, "CueRefCluster" },
    { EbmlElementId::CueRefNumber, EbmlElementType::UnsignedInteger, 5, "CueRefNumber" },
    { EbmlElementId::CueRefCodecState, EbmlElementType::UnsignedInteger, 5, "CueRefCodecState" },
    { EbmlElementId::Attachments, EbmlElementType::Master, 1, "Attachments" },
    { EbmlElementId::AttachedFile, EbmlElementType::Master, 2, "AttachedFile" },
    { EbmlElementId::FileDescription, EbmlElementType::Utf8String, 3, "FileDescription" },
    { EbmlElementId::FileName, EbmlElementType::Utf8String, 3, "FileName" },
    { EbmlElementId::FileMimeType, EbmlElementType::String, 3, "FileMimeType" },
    { EbmlElementId::FileData, EbmlElementType::Binary, 3, "FileData" },
    { EbmlElementId::FileUID, EbmlElementType::UnsignedInteger, 3, "FileUID" },
    { EbmlElementId::FileReferral, EbmlElementType::Binary, 3, "FileReferral" },
    { EbmlElementId::FileUsedStartTime, EbmlElementType::UnsignedInteger, 3, "FileUsedStartTime" },
    { EbmlElementId::FileUsedEndTime, EbmlElementType::UnsignedInteger, 3, "FileUsedEndTime" },
    { EbmlElementId::Chapters, EbmlElementType::Master, 1, "Chapters" },
    { EbmlElementId::EditionEntry, EbmlElementType::Master, 2, "EditionEntry" },
    { EbmlElementId::EditionUID, EbmlElementType::UnsignedInteger, 3, "EditionUID" },
    { EbmlElementId::EditionFlagHidden, EbmlElementType::UnsignedInteger, 3, "EditionFlagHidden" },
    { EbmlElementId::EditionFlagDefault, EbmlElementType::UnsignedInteger, 3, "EditionFlagDefault" },
    { EbmlElementId::EditionFlagOrdered, EbmlElementType::UnsignedInteger, 3, "EditionFlagOrdered" },
    { EbmlElementId::ChapterAtom, EbmlElementType::Master, 3, "ChapterAtom" },
    { EbmlElementId::ChapterUID, EbmlElementType::UnsignedInteger, 4, "ChapterUID" },
    { EbmlElementId::ChapterStringUID, EbmlElementType::Utf8String, 4, "ChapterStringUID" },
    { EbmlElementId::ChapterTimeStart, EbmlElementType::UnsignedInteger, 4, "ChapterTimeStart" },
    { EbmlElementId::ChapterTimeEnd, EbmlElementType::UnsignedInteger, 4, "ChapterTimeEnd" },
    { EbmlElementId::ChapterFlagHidden, EbmlElementType::UnsignedInteger, 4, "ChapterFlagHidden" },
    { EbmlElementId::ChapterFlagEnabled, EbmlElementType::UnsignedInteger, 4, "ChapterFlagEnabled" },
    { EbmlElementId::ChapterSegmentUID, EbmlElementType::Binary, 4, "ChapterSegmentUID" },
    { EbmlElementId::ChapterSegmentEditionUID, EbmlElementType::UnsignedInteger, 4, "ChapterSegmentEditionUID" },
    { EbmlElementId::ChapterPhysicalEquiv, EbmlElementType::UnsignedInteger, 4, "ChapterPhysicalEquiv" },
    { EbmlElementId::ChapterTrack, EbmlElementType::Master, 4, "ChapterTrack" },
    { EbmlElementId::ChapterTrackNumber, EbmlElementType::UnsignedInteger, 5, "ChapterTrackNumber" },
    { EbmlElementId::ChapterDisplay, EbmlElementType::Master, 4, "ChapterDisplay" },
    { EbmlElementId::ChapString, EbmlElementType::Utf8String, 5, "ChapString" },
    { EbmlElementId::ChapLanguage, EbmlElementType::String, 5, "ChapLanguage" },
    { EbmlElementId::ChapCountry, EbmlElementType::String, 5, "ChapCountry" },
    { EbmlElementId::ChapProcess, EbmlElementType::Master, 4, "ChapProcess" },
    { EbmlElementId::ChapProcessCodecID, EbmlElementType::UnsignedInteger, 5, "ChapProcessCodecID" },
    { EbmlElementId::ChapProcessPrivate, EbmlElementType::Binary, 5, "ChapProcessPrivate" },
    { EbmlElementId::ChapProcessCommand, EbmlElementType::Master, 5, "ChapProcessCommand" },
    { EbmlElementId::ChapProcessTime, EbmlElementType::UnsignedInteger, 6, "ChapProcessTime" },
    { EbmlElementId::ChapProcessData, EbmlElementType::Binary, 6, "ChapProcessData" },
    { EbmlElementId::Tags, EbmlElementType::Master, 1, "Tags" },
    { EbmlElementId::Tag, EbmlElementType::Master, 2, "Tag" },
    { EbmlElementId::Targets, EbmlElementType::Master, 3, "Targets" },
    { EbmlElementId::TargetTypeValue, EbmlElementType::UnsignedInteger, 4, "TargetTypeValue" },
    { EbmlElementId::TargetType, EbmlElementType::String, 4, "TargetType" },
    { EbmlElementId::TagTrackUID, EbmlElementType::UnsignedInteger, 4, "TagTrackUID" },
    { EbmlElementId::TagEditionUID, EbmlElementType::UnsignedInteger, 4, "TagEditionUID" },
    { EbmlElementId::TagChapterUID, EbmlElementType::UnsignedInteger, 4, "TagChapterUID" },
    { EbmlElementId::TagAttachmentUID, EbmlElementType::UnsignedInteger, 4, "TagAttachmentUID" },
    { EbmlElementId::SimpleTag, EbmlElementType::Master, 3, "SimpleTag" },
    { EbmlElementId::TagName, EbmlElementType::Utf8String, 4, "TagName" },
    { EbmlElementId::TagLanguage, EbmlElementType::String, 4, "TagLanguage" },
    { EbmlElementId::TagDefault, EbmlElementType::UnsignedInteger, 4, "TagDefault" },
    { EbmlElementId::TagString, EbmlElementType::Utf8String, 4, "TagString" },
    { EbmlElementId::TagBinary, EbmlElementType::Binary, 4, "TagBinary" }
};

const size_t ebml_element_descriptor_count = sizeof(ebml_element_descriptors) / sizeof(ebml_element_descriptors[0]);

EbmlElementDescriptor* get_ebml_element_descriptor(EbmlElementId ebml_element_id)
{
    size_t ebml_element_descriptor_index = 0;
    while (ebml_element_descriptor_index < ebml_element_descriptor_count)
    {
        if (ebml_element_descriptors[ebml_element_descriptor_index].id_ == ebml_element_id)
        {
            return &ebml_element_descriptors[ebml_element_descriptor_index];
        }
        ebml_element_descriptor_index++;
    }
    return nullptr;
}

size_t get_ebml_element_id_length(unsigned char first_byte_of_element_id)
{
	for (unsigned char position = 7; position > 3; position--)
	{
		if (first_byte_of_element_id & 1 << position)
		{
			return 8 - position;
		}
	}
	throw std::runtime_error("invalid element id");
}

std::string get_ebml_element_name(EbmlElementId ebml_element_id)
{
    const auto* ebml_element_descriptor = get_ebml_element_descriptor(ebml_element_id);
    if (ebml_element_descriptor)
    {
        return ebml_element_descriptor->name_;
    }
    return "<unknown element>";
}

EbmlElementType get_ebml_element_type(EbmlElementId ebml_element_id)
{
    const auto* ebml_element_descriptor = get_ebml_element_descriptor(ebml_element_id);
    if (ebml_element_descriptor)
    {
        return ebml_element_descriptor->type_;
    }
    throw std::runtime_error("unknown element");
}

int get_ebml_element_level(EbmlElementId ebml_element_id)
{
    const auto* ebml_element_descriptor = get_ebml_element_descriptor(ebml_element_id);
    if (ebml_element_descriptor)
    {
        return ebml_element_descriptor->level_;
    }
    throw std::runtime_error("unknown element");
}

EbmlElementId read_ebml_element_id(unsigned char* data, size_t& available_data_length, size_t& ebml_element_id_length)
{
	ebml_element_id_length = get_ebml_element_id_length(*data);
	if (ebml_element_id_length > available_data_length)
	{
		throw std::runtime_error(std::string("not enough data to read element id (element id length - ").append(std::to_string(ebml_element_id_length)).append(" bytes)"));
	}
	available_data_length -= ebml_element_id_length;
	unsigned char ebml_element_id_buffer[4] = { 0 };
	memcpy(&ebml_element_id_buffer[4 - ebml_element_id_length], data, ebml_element_id_length);
	auto ebml_element_id = ntohl(*reinterpret_cast<unsigned int*>(ebml_element_id_buffer));
    const auto* ebml_element_descriptor = get_ebml_element_descriptor(static_cast<EbmlElementId>(ebml_element_id));
    if (ebml_element_descriptor)
    {
        return ebml_element_descriptor->id_;
    }
    std::stringstream error;
    error << "unknown element id (0x" << std::hex << ebml_element_id << ")";
    throw std::runtime_error(error.str());
}

size_t get_ebml_element_size_length(const unsigned char* data, size_t available_data_length)
{
	for (unsigned char position = 7; position > 0; position--)
	{
		if (*data & 1 << position)
		{
			return 8 - position;
		}
	}
	if (*data == 1)
	{
		return 8;
	}
	throw std::runtime_error("invalid element size");
}

uint64_t get_ebml_element_size(const unsigned char* data, size_t available_data_length, size_t &ebml_element_size_length)
{
	ebml_element_size_length = get_ebml_element_size_length(data, available_data_length);
	if (ebml_element_size_length > available_data_length)
	{
		throw std::runtime_error(std::string("not enough data to read element size (element size length - ").append(std::to_string(ebml_element_size_length)).append(" bytes)"));
	}
	available_data_length -= ebml_element_size_length;
	unsigned char element_size[8] = { 0 };
	memcpy(&element_size[8 - ebml_element_size_length], data, ebml_element_size_length);
	element_size[8 - ebml_element_size_length] -= 1 << (8 - ebml_element_size_length);
	if (ebml_element_size_length == 1 && element_size[7] == 0x7f)
	{
		return -1;
	}
	return ntohll(*reinterpret_cast<uint64_t*>(element_size));
}

std::string get_ebml_element_value(EbmlElementId id, EbmlElementType type, unsigned char* data, uint64_t size)
{
	switch (type)
	{
	case EbmlElementType::UnsignedInteger:
	case EbmlElementType::SignedInteger:
	{
		unsigned char Integer[8] = { 0 };
		memcpy(&Integer[8 - size], data, static_cast<size_t>(size));
		return std::to_string(type == EbmlElementType::UnsignedInteger ? ntohll(*reinterpret_cast<uint64_t*>(Integer)) : static_cast<long long>(ntohll(*reinterpret_cast<uint64_t*>(Integer))));
	}
	case EbmlElementType::String:
	case EbmlElementType::Utf8String:
		return std::string(reinterpret_cast<char*>(data), static_cast<unsigned int>(size));
	case EbmlElementType::Float:
		return std::to_string(size == 4 ? ntohf(*reinterpret_cast<unsigned int*>(data)) : ntohd(*reinterpret_cast<unsigned long long*>(data)));
	case EbmlElementType::Date:
	{
		time_t timestamp = 978307200 + ntohll(*reinterpret_cast<uint64_t*>(data)) / 100000000;
		char buffer[20];
		tm date_and_time;
#if defined(_WIN32)
		gmtime_s(&date_and_time, &timestamp);
#else
        gmtime_r(&timestamp, &date_and_time);
#endif
		strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", &date_and_time);
		return std::string(buffer);
	}
	case EbmlElementType::Binary:
	{
		switch (id)
		{
		case EbmlElementId::SimpleBlock:
		{
			size_t track_number_size_length;
			auto track_number = get_ebml_element_size(data, static_cast<size_t>(size), track_number_size_length);
			char buffer[56];
#if defined(_WIN32)
			sprintf_s(buffer, "track %" PRIu64 ", timecode %d, flags 0x%02x", track_number, ntohs(*reinterpret_cast<short*>(data + track_number_size_length)), *(data + track_number_size_length + 2));
#else
			sprintf(buffer, "track %" PRIu64 ", timecode %d, flags 0x%02x", track_number, ntohs(*reinterpret_cast<short*>(data + track_number_size_length)), *(data + track_number_size_length + 2));
#endif
			return std::string(buffer);
		}
		case EbmlElementId::SeekID:
		{
			size_t available_data_length = size, id_size;
			EbmlElementId id = read_ebml_element_id(data, available_data_length, id_size);
			return std::to_string(static_cast<unsigned int>(id));
		}
		}
	}
	default:
		return "<cannot parse the content of this element>";
	}
}

EbmlElement::EbmlElement(EbmlElementId id, EbmlElementType type, uint64_t size, size_t id_length, size_t size_length, unsigned char* data) : id_(id), type_(type), size_(size), id_length_(id_length), size_length_(size_length), data_(data)
{
}

void EbmlElement::add_child(EbmlElement child)
{
	children_.push_back(child);
}

void EbmlElement::print(unsigned int level)
{
	std::string indent;
	if (level > 0)
	{
		std::cout << "|" << std::string(level - 1, ' ');
	}
	std::cout << "+ " << get_ebml_element_name(id_) << " (" << size_ << ")";
	if (type_ != EbmlElementType::Master)
	{
		std::cout << ": " << get_ebml_element_value(id_, type_, data_, size_);
	}
	std::cout << std::endl;
	for (EbmlElement& child : children_)
	{
		child.print(level + 1);
	}
}

uint64_t EbmlElement::size()
{
	return size_;
}

uint64_t EbmlElement::element_size()
{
	return id_length_ + size_length_ + size_;
}

void EbmlElement::calculate_size()
{
    uint64_t size = 0;
	for (auto& child : children_)
	{
		size += child.element_size();
	}
	size_ = size;
}

EbmlElementId EbmlElement::id() const
{
	return id_;
}

const std::list<EbmlElement>& EbmlElement::children() const
{
	return children_;
}

const std::string EbmlElement::value() const
{
	if (type_ != EbmlElementType::Master)
	{
		return get_ebml_element_value(id_, type_, data_, size_);
	}
	throw std::runtime_error("cannot obtain a value from a master element");
}

const unsigned char* EbmlElement::data() const
{
	return data_;
}

uint64_t EbmlElement::size() const
{
	return size_;
}

EbmlElementType EbmlElement::type() const
{
    return type_;
}

const EbmlElement* EbmlElement::first_child(EbmlElementId id) const
{
    auto first_child = std::find_if(children_.begin(), children_.end(), [id](const EbmlElement& child) { return child.id() == id; });
    if (first_child != children_.end())
    {
        return &(*first_child);
    }
    return nullptr;
}

std::vector<const EbmlElement*> EbmlElement::children(EbmlElementId id) const
{
    std::vector<const EbmlElement*> result;
    children(result, id);
    return result;
}

void EbmlElement::children(std::vector<const EbmlElement*>& children, EbmlElementId id) const
{
    for (const auto& child : children_)
    {
        if (child.id() == id)
        {
            children.emplace_back(&child);
        }
    }
}

std::vector<const EbmlElement*> EbmlElement::descendants(EbmlElementId id) const
{
    std::vector<const EbmlElement*> result;
    descendants(result, id);
    return result;
}

void EbmlElement::descendants(std::vector<const EbmlElement*>& descendants, EbmlElementId id) const
{
    children(descendants, id);
    for (const auto& child  : children_)
    {
        child.descendants(descendants, id);
    }
}
