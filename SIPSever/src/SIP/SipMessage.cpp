#include "SipMessage.hpp"
#include "SipMessageTypes.h"
#include "SipMessageHeaders.h"
#include <vector>
#include <string>
#include <cstring>
#include "Log.hpp"

SipMessage::SipMessage(std::string message, sockaddr_in src) : _messageStr(std::move(message)), _src(std::move(src))
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
			_via = line;
		}
		else if (line.find(SipMessageHeaders::FROM) != std::string::npos)
		{
			_from = line;
			_fromNumber = extractNumber(line);
		}
		else if (line.find(SipMessageHeaders::TO) == 0)
		{
			_to = line;
			_toNumber = extractNumber(line);
		}
		else if (line.find(SipMessageHeaders::CALL_ID) != std::string::npos)
		{
			_callID = line;
		}
		else if (line.find(SipMessageHeaders::CSEQ) != std::string::npos)
		{
			_cSeq = line;
		}
		else if (line.find(SipMessageHeaders::CONTACT) != std::string::npos)
		{
			_contact = line;
			_contactNumber = extractNumber(std::move(line));
		}
		else if (line.find(SipMessageHeaders::CONTENT_LENGTH) != std::string::npos)
		{
			_contentLength = line;
		} 
		else if (line.find(SipMessageHeaders::REFER_TO) != std::string::npos)
		{
			_referTo = line;
			_referToNumber = extractNumber(std::move(line));
		} 
		else if (line.find(SipMessageHeaders::REFERED_BY) != std::string::npos)
		{
			_referedBy = line;
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
	auto toPos = _messageStr.find(_to);
	_messageStr.replace(toPos, _to.length(), value);
	_to = value;
	_toNumber = extractNumber(std::move(value));;
}

void SipMessage::setCallID(std::string value)
{
	auto callIdPos = _messageStr.find(_callID);
	_messageStr.replace(callIdPos, _callID.length(), value);
	_callID = std::move(value);
}

void SipMessage::setCSeq(std::string value)
{
	auto cSeqPos = _messageStr.find(_cSeq);
	_messageStr.replace(cSeqPos, _cSeq.length(), value);
	_cSeq = std::move(value);
}

void SipMessage::setContact(std::string value)
{
	auto contactPos = _messageStr.find(_contact);
	_messageStr.replace(contactPos, _contact.length(), value);
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

void SipMessage::setContentLength(std::string value)
{
	auto contentLengthPos = _messageStr.find(_contentLength);
	_messageStr.replace(contentLengthPos, _contentLength.length(), value);
	_contentLength = std::move(value);
}

std::string SipMessage::toString() const
{
	return _messageStr;
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

std::string SipMessage::getBranch() const
{
        return _via.substr(_via.find("branch=") + 8, _via.find(";") - _via.find("branch=") - 8);
}

std::string SipMessage::getFrom() const
{
	return _from;
}

std::string SipMessage::getFromNumber() const
{
	return _fromNumber;
}

std::string SipMessage::getTo() const
{
	return _to;
}

std::string SipMessage::getToNumber() const
{
	return _toNumber;
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

sockaddr_in SipMessage::getSource() const
{
	return _src;
}

std::string SipMessage::getContentLength() const
{
	return _contentLength;
}

std::string SipMessage::extractNumber(std::string header) const
{
	auto indexOfNumber = header.find("sip:") + 4;
	return header.substr(indexOfNumber, header.find("@") - indexOfNumber);
}

std::string SipMessage::dump() 
{
	return "SipMessage{\n" \
		"\tType: " + _type + "\n " \
		"\tHeader: " + _header + "\n " \
		"\tVia: " + _via + "\n " \
		"\tFrom: " + _from + "\n " \
		"\tTo: " + _to + "\n " \
		"\tCallID: " + _callID + "\n " \
		"\tCSeq: " + _cSeq + "\n " \
		"\tContact: " + _contact + "\n " \
		"\tContentLength: " + _contentLength + "\n " \
		"\tReferTo: " + _referTo + "\n " \
		"\tReferToNumber: " + _referToNumber + "\n " \
		"\tReferedBy: " + _referedBy + "\n " \
		"\tSource: " + std::to_string(_src.sin_addr.s_addr) + ":" + std::to_string(_src.sin_port) + "\n}";
}
