#pragma once

#define ASIO_STANDALONE

#include "asio-1.10.8\include\asio.hpp"
#include <iostream>

using namespace asio::ip;

enum ExecutableType {
	UI,      //for the front end contacting the service
	SERVICE, //for the CageService
	MANAGER  //for the CageManager
};

class NetworkManager {
	asio::io_service ioservice;
	tcp::socket socket;

	tcp::endpoint send_endpoint;
	tcp::endpoint send_endpoint_2; //this is only used by service to give feedback to ui - not implemented so far
	tcp::endpoint rec_endpoint;

	tcp::acceptor acceptor; //accepting tcp connections

	std::vector<char> send_buf;
	std::vector<char> rec_buf; //obsolete

public:
	/*
	* Constructor
	* type variable says in what role network manager should be initialized
	*/
	NetworkManager(ExecutableType type): ioservice(), socket(ioservice), acceptor(ioservice, tcp::endpoint(tcp::v4(), 1338)) {
		switch (type){
		case UI:
			initUI();
			break;
		case SERVICE:
			initSERVICE();
			break;
		case MANAGER:
			initMANAGER();
			break;
		default:
			break;
		}
	}


/**
* DO NOT USE - OBSOLETE FUNCTION
* Function to be used by UI and cageManager to listen for messages from service
* max lenth of message is 1024 characters  - evey message must end with '\n' character
**/
	std::string recieve(){
		rec_buf.clear();
		rec_buf.resize(1024);
		try {
			size_t len = socket.receive(asio::buffer(rec_buf));
			rec_buf.resize(len);
		}
		catch (std::system_error e) {
			std::cout << e.what();
		}
		return toString(rec_buf);
	}


/**
* Function to be used to send messages
* max message lenght is 1024 characters
*
**/
	bool send(std::string msg) {

		std::vector<char> message = toCharVector(msg);

		try {
			tcp::socket tmp_socket(ioservice);
			tmp_socket.connect(send_endpoint);
			send_buf = message;
			tmp_socket.write_some(asio::buffer(send_buf));
			return true;
		}
		catch (std::exception& ex) {
			return false;
		}
	}

/**
* function used by all components to listen for messages
*  #blocking call
*
**/
	std::string listen() {
		tcp::socket temp_socket(ioservice);
		acceptor.accept(temp_socket);
		asio::streambuf  buffer;

		try {
			size_t len = asio::read_until(temp_socket, buffer,'\n');
		}
		catch (std::system_error e) {
			std::cout << e.what();
		}

		std::istream str(&buffer);
		std::string s;
		std::getline(str, s);

		return s;

	}

private:

//help functions, not important for developers
	// ports lisened to: ui 1337, service 1338, manager 1339

	bool initUI() {
		//tcp::resolver resolver(ioservice);
		//tcp::resolver::query query(tcp::v4(), "localhost", "1337");
		//rec_endpoint = *resolver.resolve(query);

		//socket.close();
		//socket.connect(rec_endpoint);

		tcp::resolver resolver2(ioservice);
		tcp::resolver::query query2(tcp::v4(), "localhost", "1338");
		send_endpoint = *resolver2.resolve(query2);

		//acceptor = tcp::acceptor(ioservice, tcp::endpoint(tcp::v4(), 1337));

		return true;
	}
	bool initSERVICE() {
		tcp::resolver resolver(ioservice);
		tcp::resolver::query query2(tcp::v4(), "localhost", "1339");
		send_endpoint = *resolver.resolve(query2);


		acceptor = tcp::acceptor(ioservice, tcp::endpoint(tcp::v4(), 1338));

		return true;
	}//TODO
	bool initMANAGER() {
		acceptor = tcp::acceptor(ioservice, tcp::endpoint(tcp::v4(), 1339));

		return true;
	}; //TODO

	std::string toString(std::vector<char> message) {

		std::string _string;
		for (char &c : message) {
			_string += c;
		}
		return _string;
	}

	std::vector<char> toCharVector(std::string const &_string) {
		std::vector<char> message;
		for (char const & c : _string) {
			message.push_back(c);
		}
		message.push_back('\n');
		return message;
	}
};
