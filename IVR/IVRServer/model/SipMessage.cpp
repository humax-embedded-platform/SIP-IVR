#include "SipMessage.hpp"
#include "SipMessageTypes.h"
#include "SipMessageHeaders.h"
#include <vector>
#include <string>
#include <cstring>
#include "Log.hpp"
#include "AppDefines.h"
#include "NetUtil.h"

SipMessage::SipMessage()
{

}

SipMessage::SipMessage(std::string message) : _messageStr(std::move(message))
{
	parse();
}

void SipMessage::parse()
{
	std::string msg = _messageStr;

	size_t pos = msg.find(SipMessageHeaders::HEADERS_DELIMETER);
	_header = msg.substr(0, pos);
	msg.erase(0, pos + std::strlen(SipMessageHeaders::HEADERS_DELIMETER));

	_type = _header.substr(0, _header.find(" "));
	if (_type == "SIP/2.0")
	{
		_type = _header;
	}

	while ((pos = msg.find(SipMessageHeaders::HEADERS_DELIMETER)) != std::string::npos) {
		std::string line = msg.substr(0, pos);

		if (line.find(SipMessageHeaders::VIA) != std::string::npos)
		{
            _via = extractHeader(line, SipMessageHeaders::VIA);
            extractSrc(line, _src);
		}
		else if (line.find(SipMessageHeaders::FROM) != std::string::npos)
		{
            _from = extractHeader(line, SipMessageHeaders::FROM);
			_fromNumber = extractNumber(line);
            _fromTag = extractTag(line);
		}
		else if (line.find(SipMessageHeaders::TO) == 0)
		{
            _to = extractHeader(line, SipMessageHeaders::TO);
			_toNumber = extractNumber(line);
            _toTag = extractTag(line);
		}
		else if (line.find(SipMessageHeaders::CALL_ID) != std::string::npos)
		{
            _callID = extractHeader(line, SipMessageHeaders::CALL_ID);
		}
		else if (line.find(SipMessageHeaders::CSEQ) != std::string::npos)
		{
            _cSeq = extractHeader(line, SipMessageHeaders::CSEQ);
		}
		else if (line.find(SipMessageHeaders::CONTACT) != std::string::npos)
		{
            _contact = extractHeader(line, SipMessageHeaders::CONTACT);
			_contactNumber = extractNumber(std::move(line));
		}
		else if (line.find(SipMessageHeaders::CONTENT_LENGTH) != std::string::npos)
		{
            _contentLength = extractHeader(line, SipMessageHeaders::CONTENT_LENGTH);
		} 
		else if (line.find(SipMessageHeaders::REFER_TO) != std::string::npos)
		{
            _referTo = extractHeader(line, SipMessageHeaders::REFER_TO);
			_referToNumber = extractNumber(std::move(line));
		} 
		else if (line.find(SipMessageHeaders::REFERED_BY) != std::string::npos)
		{
            _referedBy = extractHeader(line, SipMessageHeaders::REFERED_BY);
		}


		msg.erase(0, pos + std::strlen(SipMessageHeaders::HEADERS_DELIMETER));
	}
	if (!isValidMessage())
	{
		throw std::runtime_error("Invalid message.");
	}
}

bool SipMessage::isValidMessage() const
{
	if (_via.empty() || _to.empty() || _from.empty() || _callID.empty() || _cSeq.empty())
	{
		return false;
	}

	if ((_type == SipMessageTypes::INVITE || _type == SipMessageTypes::REGISTER) && _contact.empty())
	{
		return false;
	}

	return true;
}

std::string SipMessage::extractHeader(std::string &line, std::string header) const
{
    std::string partern = std::string(header) + ": ";
    auto indexOfHeader = line.find(partern);
    if (indexOfHeader != 0)
    {
        return "";
    }
    return line.substr(indexOfHeader + partern.length(), line.find(SipMessageHeaders::HEADERS_DELIMETER));
}

void SipMessage::setType(std::string value)
{
	_type = std::move(value);
}

void SipMessage::setHeader(std::string value)
{
	auto headerPos = _messageStr.find(_header);
	_messageStr.replace(headerPos, _header.length(), value);
	_header = std::move(value);
}

void SipMessage::setVia(std::string value)
{
	auto viaPos = _messageStr.find(_via);
	_messageStr.replace(viaPos, _via.length(), value);
	_via = std::move(value);
}

void SipMessage::setFrom(std::string value)
{
	auto fromPos = _messageStr.find(_from);
	_messageStr.replace(fromPos, _from.length(), value);
	_from = value;
	_fromNumber = extractNumber(std::move(value));
}

void SipMessage::setTo(std::string value)
{
	_to = value;
	_toNumber = extractNumber(std::move(value));;
}

void SipMessage::setCallID(std::string value)
{
	_callID = std::move(value);
}

void SipMessage::setCSeq(std::string value)
{
	_cSeq = std::move(value);
}

void SipMessage::setContact(std::string value)
{
	_contact = std::move(value);
}

void SipMessage::setReplaces(std::string value)
{
	auto replacePos = _messageStr.find(_replaces);
	if (_replaces.empty() || replacePos == std::string::npos)
	{
		_messageStr += value + SipMessageHeaders::HEADERS_DELIMETER;
	}
	else {
		_messageStr.replace(replacePos, _replaces.length(), value);
		_replaces = std::move(value);
	}
}

void SipMessage::setReferTo(std::string value)
{
    _referTo = std::move(value);
}

void SipMessage::setReferBy(std::string value)
{
    _referedBy = std::move(value);
}

void SipMessage::setContentLength(std::string value)
{
	_contentLength = std::move(value);
}

void SipMessage::setContentType(std::string value)
{
    _contentType = std::move(value);
}

std::string SipMessage::toPayload()
{
    std::string message = _header + SipMessageHeaders::HEADERS_DELIMETER + \
                          SipMessageHeaders::VIA + ": " + _via + SipMessageHeaders::HEADERS_DELIMETER + \
                          SipMessageHeaders::FROM + ": " + _from + SipMessageHeaders::HEADERS_DELIMETER + \
                          SipMessageHeaders::TO + ": " + _to + SipMessageHeaders::HEADERS_DELIMETER + \
                          SipMessageHeaders::CALL_ID + ": " + _callID + SipMessageHeaders::HEADERS_DELIMETER + \
                          SipMessageHeaders::CSEQ + ": " + _cSeq + SipMessageHeaders::HEADERS_DELIMETER + \
                          SipMessageHeaders::CONTACT + ": " + _contact + SipMessageHeaders::HEADERS_DELIMETER + \
                          SipMessageHeaders::CONTENT_TYPE + ": " + _contentType + SipMessageHeaders::HEADERS_DELIMETER + \
                          "Expires: 60" + SipMessageHeaders::HEADERS_DELIMETER + \
                          "Max-Forwards: 70" + SipMessageHeaders::HEADERS_DELIMETER + \
                          "Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO" + SipMessageHeaders::HEADERS_DELIMETER + \
                          "User-Agent: " + APP_USERAGENT + SipMessageHeaders::HEADERS_DELIMETER + \
                          (_referTo.empty() ? "" : (std::string(SipMessageHeaders::REFER_TO) + ": " + _referTo + SipMessageHeaders::HEADERS_DELIMETER)) + \
                          (_referedBy.empty() ? "" : (std::string(SipMessageHeaders::REFERED_BY) + ": " + _referedBy + SipMessageHeaders::HEADERS_DELIMETER)) + \
                           "Content-Length: " + _contentLength + SipMessageHeaders::HEADERS_DELIMETER + SipMessageHeaders::HEADERS_DELIMETER;
    return message;
}

std::string SipMessage::getType() const
{
	return _type;
}

std::string SipMessage::getHeader() const
{
	return _header;
}

std::string SipMessage::getVia() const
{
	return _via;
}

std::string SipMessage::getFrom() const
{
	return _from;
}

std::string SipMessage::getFromNumber() const
{
	return _fromNumber;
}

std::string SipMessage::getFromTag() const
{
    return _fromTag;
}

std::string SipMessage::getTo() const
{
	return _to;
}

std::string SipMessage::getToNumber() const
{
	return _toNumber;
}

std::string SipMessage::getToTag() const
{
    return _toTag;
}

std::string SipMessage::getCallID() const
{
	return _callID;
}

std::string SipMessage::getCSeq() const
{
	return _cSeq;
}

std::string SipMessage::getContact() const
{
	return _contact;
}

std::string SipMessage::getContactNumber() const
{
	return _contactNumber;
}

std::string SipMessage::getReferToNumber() const
{
	return _referToNumber;
}

std::string SipMessage::getContentLength() const
{
	return _contentLength;
}

std::string SipMessage::getContentType() const
{
    return _contentType;
}

sockaddr_in SipMessage::getSrc() const
{
    return _src;
}

std::string SipMessage::extractNumber(std::string header) const
{
	auto indexOfNumber = header.find("sip:") + 4;
	return header.substr(indexOfNumber, header.find("@") - indexOfNumber);
}

std::string SipMessage::extractTag(std::string line) const
{
    auto indexOfTag = line.find("tag=") + 4;
    return line.substr(indexOfTag, line.find(SipMessageHeaders::HEADERS_DELIMETER) - indexOfTag);
}

bool SipMessage::extractSrc(std::string line, sockaddr_in &sa)
{
    auto indexOfIP = line.find("Via: SIP/2.0/UDP ") + 18;
    auto indexOfPort = line.find(":", indexOfIP);
    auto indexOfEnd = line.find(";", indexOfPort);
    auto ip = line.substr(indexOfIP, indexOfPort - indexOfIP);
    auto port = line.substr(indexOfPort + 1, indexOfEnd - indexOfPort - 1);
    return NetUtil::ipv4ToSa(ip.c_str(), _src, std::stoi(port));
}
