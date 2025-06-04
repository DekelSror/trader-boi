"""
Timeseries Database Abstraction Layer

This package provides a unified interface for different timeseries database backends.
"""

from abc import ABC, abstractmethod


class Timeseries(ABC):
    """Abstract base class for timeseries database implementations"""
    @abstractmethod
    def write_points(self, points: tuple[int, float] | list[tuple[int, float]]) -> bool:
        """Write multiple points to the database"""
        pass
    
    @abstractmethod
    def range(self, start_time: int, end_time: int):
        """
        Query the database for points within a time range
        
        Args:
            start_time: Start time (Unix timestamp in seconds)
            end_time: End time (Unix timestamp in seconds)
            
        """
        pass
    
    @abstractmethod
    def close(self):
        """Close the database connection"""
        pass


class TimeseriesDB:
    def __init__(self, ts_class: type[Timeseries]) -> None:
        self.ts_class = ts_class
        self.series = {}
    

    def get_series(self, series_name: str) -> Timeseries:
        if not self.series.get(series_name):
            self.series[series_name] = self.ts_class()
            
        return self.series[series_name]
        

        
