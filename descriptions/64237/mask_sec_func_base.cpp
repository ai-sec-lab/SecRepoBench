std::string NtpLayer::getReferenceIdentifierString() const
    {
        uint8_t stratum = getStratum();
        uint8_t version = getVersion();
        uint32_t refID = getReferenceIdentifier();

        if (stratum == 0)
        {
            switch (version)
            {
            case 3:
            {
                switch (static_cast<ClockSource>(refID))
                {
                case ClockSource::DCN:
                    return "DCN routing protocol";
                case ClockSource::NIST:
                    return "NIST public modem";
                case ClockSource::TSP:
                    return "TSP time protocol";
                case ClockSource::DTS:
                    return "Digital Time Service";
                default:
                    return "Unknown";
                }
            }
            case 4:
            {
                switch (static_cast<KissODeath>(refID))
                {
                case KissODeath::ACST:
                    return "The association belongs to a anycast server";
                case KissODeath::AUTH:
                    return "Server authentication failed";
                case KissODeath::AUTO:
                    return "Autokey sequence failed";
                case KissODeath::BCST:
                    return "The association belongs to a broadcast server";
                case KissODeath::CRYP:
                    return "Cryptographic authentication or identification failed";
                case KissODeath::DENY:
                    return "Access denied by remote server";
                case KissODeath::DROP:
                    return "Lost peer in symmetric mode";
                case KissODeath::RSTR:
                    return "Access denied due to local policy";
                case KissODeath::INIT:
                    return "The association has not yet synchronized for the first time";
                case KissODeath::MCST:
                    return "The association belongs to a manycast server";
                case KissODeath::NKEY:
                    return "No key found.  Either the key was never installed or is not trusted";
                case KissODeath::RATE:
                    return "Rate exceeded.  The server has temporarily denied access because the client exceeded the rate "
                        "threshold";
                case KissODeath::RMOT:
                    return "Somebody is tinkering with the association from a remote host running ntpdc.  Not to worry "
                        "unless some rascal has stolen your keys";
                case KissODeath::STEP:
                    return "A step change in system time has occurred, but the association has not yet resynchronized";
                default:
                {
                    // <MASK>
                }
                }
            }
            }
        }
        else if (stratum == 1)
        {
            switch (version)
            {
            case 3:
            {
                switch (static_cast<ClockSource>(refID))
                {
                case ClockSource::ATOM:
                    return "Atomic clock";
                case ClockSource::VLF:
                    return "VLF radio";
                case ClockSource::LORC:
                    return "LORAN-C radionavigation";
                case ClockSource::GOES:
                    return "GOES UHF environment satellite";
                case ClockSource::GPS:
                    return "GPS UHF satellite positioning";
                default:
                    return "Unknown";
                }
            }
            case 4:
            {
                switch (static_cast<ClockSource>(refID))
                {
                case ClockSource::GOES:
                    return "Geosynchronous Orbit Environment Satellite";
                case ClockSource::GPS:
                    return "Global Position System";
                case ClockSource::GAL:
                    return "Galileo Positioning System";
                case ClockSource::PPS:
                    return "Generic pulse-per-second";
                case ClockSource::IRIG:
                    return "Inter-Range Instrumentation Group";
                case ClockSource::WWVB:
                    return "LF Radio WWVB Ft. Collins, CO 60 kHz";
                case ClockSource::DCF:
                    return "LF Radio DCF77 Mainflingen, DE 77.5 kHz";
                case ClockSource::HBG:
                    return "LF Radio HBG Prangins, HB 75 kHz";
                case ClockSource::MSF:
                    return "LF Radio MSF Anthorn, UK 60 kHz";
                case ClockSource::JJY:
                    return "LF Radio JJY Fukushima, JP 40 kHz, Saga, JP 60 kHz";
                case ClockSource::LORC:
                    return "MF Radio LORAN C station, 100 kHz";
                case ClockSource::TDF:
                    return "MF Radio Allouis, FR 162 kHz";
                case ClockSource::CHU:
                    return "HF Radio CHU Ottawa, Ontario";
                case ClockSource::WWV:
                    return "HF Radio WWV Ft. Collins, CO";
                case ClockSource::WWVH:
                    return "HF Radio WWVH Kauai, HI";
                case ClockSource::NIST:
                    return "NIST telephone modem";
                case ClockSource::ACTS:
                    return "NIST telephone modem";
                case ClockSource::USNO:
                    return "USNO telephone modem";
                case ClockSource::PTB:
                    return "European telephone modem";
                case ClockSource::MRS:
                    return "Multi Reference Sources";
                case ClockSource::XFAC:
                    return "Inter Face Association Changed";
                case ClockSource::STEP:
                    return "Step time change";
                case ClockSource::GOOG:
                    return "Google NTP servers";
                case ClockSource::DCFa:
                    return "Meinberg DCF77 with amplitude modulation";
                case ClockSource::DCFp:
                    return "Meinberg DCF77 with phase modulation)/pseudo random phase modulation";
                case ClockSource::GPSs:
                    return "Meinberg GPS (with shared memory access)";
                case ClockSource::GPSi:
                    return "Meinberg GPS (with interrupt based access)";
                case ClockSource::GLNs:
                    return "Meinberg GPS/GLONASS (with shared memory access)";
                case ClockSource::GLNi:
                    return "Meinberg GPS/GLONASS (with interrupt based access)";
                case ClockSource::LCL:
                    return "Meinberg Undisciplined local clock";
                case ClockSource::LOCL:
                    return "Meinberg Undisciplined local clock";
                default:
                    return "Unknown";
                }
            }
            }
        }
        else
        {
            // TODO: Support IPv6 cases for NTPv4, it equals to MD5 hash of first four octets of IPv6 address

            pcpp::IPv4Address addr(getReferenceIdentifier());
            return addr.toString();
        }

        PCPP_LOG_ERROR("Unknown Stratum type");
        return std::string();
    }