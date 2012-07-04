//LiveInfo.h

#ifndef _PPBOX_CDN_LIVE_INFO_H_
#define _PPBOX_CDN_LIVE_INFO_H_

#define MAX_SEGMENT_SIZS 360

namespace ppbox
{
    namespace cdn
    {
        class LiveSegmentsInfo
        {
        public:
            struct SegmentInfo
            {
                SegmentInfo()
                {
                    clear();
                }

                SegmentInfo(size_t s)
                    : segment(s)
                {
                }

                void clear()
                {
                    head_leng = -1;
                    file_size = -1;
                    duration = 5;
                    segment = -1;
                }
                boost::uint64_t head_leng;
                boost::uint64_t file_size;
                boost::uint32_t duration;
                size_t segment;
            };
        public:
            LiveSegmentsInfo()
            {
            }

            ~LiveSegmentsInfo()
            {
            }

            void add_segment(size_t segment)
            {
                if (segments_.size() >= MAX_SEGMENT_SIZS)
                {
                    segments_.pop_front();
                }

                do 
                {
                    if (segments_.size() < 1) 
                    {
                        break;
                    }

                    if (segment < (*segments_.begin()).segment)
                    {
                        clear();
                        break;
                    }

                    if (segment > (segments_.back().segment+1))
                    {
                        clear();
                        break;
                    }

                    if (segment == (segments_.back().segment+1))
                    {
                        break;
                    }

                    return;

                } while (false);

                segments_.push_back(SegmentInfo(segment));

            }

            SegmentInfo& operator [](size_t segment)
            {
                if (segments_.size() < 1 || segment < (*segments_.begin()).segment || segment > segments_.back().segment)
                {
                    default_.clear();
                    return default_;
                }
                return *(segments_.begin() + (segment - (*segments_.begin()).segment));
            }

        private:
            void clear()
            {
                segments_.clear();
            }

        private:
            std::deque<SegmentInfo> segments_;
            SegmentInfo default_;

        };
    }//cdn
}//ppbox

#endif//_PPBOX_CDN_LIVE_INFO_H_

//LiveInfo.h