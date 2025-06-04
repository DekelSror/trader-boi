#!/usr/bin/env python3

"""
Market Data Processing System Control Tool

This tool allows you to manage aggregations and algorithms in the system.
"""

import os
import sys
import json
import argparse
import zmq
import time

# Configuration
ZMQ_CTRL_ADDRESS = os.environ.get("ZMQ_CTRL_ADDRESS", "tcp://127.0.0.1:5556")


def send_command(command):
    """Send a command to the backend and get the response"""
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect(ZMQ_CTRL_ADDRESS)
    
    print(f"Sending command: {json.dumps(command)}")
    socket.send_json(command)
    
    # Wait for response with timeout
    if socket.poll(5000):  # 5 second timeout
        response = socket.recv_json()
        print(f"Response: {json.dumps(response)}")
        return response
    else:
        print("Error: No response from server (timeout)")
        return {"status": "error", "message": "Timeout waiting for response"}
    

def list_aggregations():
    """List all registered aggregations"""
    command = {
        "type": "aggregation",
        "action": "list"
    }
    return send_command(command)


def add_aggregation(args):
    """Add a new aggregation"""
    # Parse config from args
    config = {}
    if args.config:
        for item in args.config:
            key, value = item.split('=')
            try:
                # Try to convert to appropriate type
                if value.isdigit():
                    value = int(value)
                elif value.replace('.', '', 1).isdigit():
                    value = float(value)
                elif value.lower() in ('true', 'false'):
                    value = value.lower() == 'true'
            except:
                pass
            config[key] = value
    
    command = {
        "type": "aggregation",
        "action": "add",
        "aggregation_type": args.type,
        "name": args.name,
        "config": config
    }
    return send_command(command)


def remove_aggregation(args):
    """Remove an aggregation"""
    command = {
        "type": "aggregation",
        "action": "remove",
        "name": args.name
    }
    return send_command(command)


def list_algorithms():
    """List all registered algorithms"""
    command = {
        "type": "algorithm",
        "action": "list"
    }
    return send_command(command)


def add_algorithm(args):
    """Add a new algorithm"""
    # Parse config from args
    config = {}
    if args.config:
        for item in args.config:
            key, value = item.split('=')
            try:
                # Try to convert to appropriate type
                if value.isdigit():
                    value = int(value)
                elif value.replace('.', '', 1).isdigit():
                    value = float(value)
                elif value.lower() in ('true', 'false'):
                    value = value.lower() == 'true'
            except:
                pass
            config[key] = value
    
    command = {
        "type": "algorithm",
        "action": "add",
        "algorithm_type": args.type,
        "name": args.name,
        "path": args.path,
        "config": config
    }
    return send_command(command)


def remove_algorithm(args):
    """Remove an algorithm"""
    command = {
        "type": "algorithm",
        "action": "remove",
        "name": args.name
    }
    return send_command(command)


def restart_algorithm(args):
    """Restart an algorithm"""
    command = {
        "type": "algorithm",
        "action": "restart",
        "name": args.name
    }
    return send_command(command)


def main():
    # Main parser
    parser = argparse.ArgumentParser(description="Market Data Processing System Control Tool")
    subparsers = parser.add_subparsers(dest="command", help="Command to execute")
    
    # Aggregation commands
    agg_list = subparsers.add_parser("agg-list", help="List all aggregations")
    
    agg_add = subparsers.add_parser("agg-add", help="Add a new aggregation")
    agg_add.add_argument("--type", required=True, help="Aggregation type (e.g., ohlc)")
    agg_add.add_argument("--name", required=True, help="Unique name for the aggregation")
    agg_add.add_argument("--config", nargs="+", help="Configuration key=value pairs")
    
    agg_remove = subparsers.add_parser("agg-remove", help="Remove an aggregation")
    agg_remove.add_argument("--name", required=True, help="Name of the aggregation to remove")
    
    # Algorithm commands
    alg_list = subparsers.add_parser("alg-list", help="List all algorithms")
    
    alg_add = subparsers.add_parser("alg-add", help="Add a new algorithm")
    alg_add.add_argument("--type", required=True, choices=["python", "so", "dll"], help="Algorithm type")
    alg_add.add_argument("--name", required=True, help="Unique name for the algorithm")
    alg_add.add_argument("--path", required=True, help="Path to the algorithm file")
    alg_add.add_argument("--config", nargs="+", help="Configuration key=value pairs")
    
    alg_remove = subparsers.add_parser("alg-remove", help="Remove an algorithm")
    alg_remove.add_argument("--name", required=True, help="Name of the algorithm to remove")
    
    alg_restart = subparsers.add_parser("alg-restart", help="Restart an algorithm")
    alg_restart.add_argument("--name", required=True, help="Name of the algorithm to restart")
    
    # Parse arguments
    args = parser.parse_args()
    
    # Execute command
    if args.command == "agg-list":
        list_aggregations()
    elif args.command == "agg-add":
        add_aggregation(args)
    elif args.command == "agg-remove":
        remove_aggregation(args)
    elif args.command == "alg-list":
        list_algorithms()
    elif args.command == "alg-add":
        add_algorithm(args)
    elif args.command == "alg-remove":
        remove_algorithm(args)
    elif args.command == "alg-restart":
        restart_algorithm(args)
    else:
        parser.print_help()


if __name__ == "__main__":
    main() 