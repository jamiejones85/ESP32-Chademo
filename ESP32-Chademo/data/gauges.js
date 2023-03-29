var chartSoc, chartTemp, chartCurrent, chartChargerCurrent, chartChargerTemp, chartChargerVolts

var updateGauge = function(key, value) {

    if (key == 'voltage') {
        updateVoltage(value)
    } else if (key == 'amperage') {
        updateCurrent(value)
    } else if (key == 'power') {
        updatePower(value)
    } else if (key == 'soc') {
        updateSOC(value)
    }

};

var updateSOC = function(newVal) {
    if (chartSoc) {
        point = chartSoc.series[0].points[0];
        point.update(Math.round(newVal));
    }
}


var updateCurrent = function(newVal) {
    if (chartCurrent) {
        point = chartCurrent.series[0].points[0];
        point.update(Math.round(newVal * -1));
    }
}

var updatePower = function(newVal) {
    if (chartPower) {
        point = chartPower.series[0].points[0];
        point.update(Math.round(newVal * -1));
    }
}

var updateVoltage = function(newVal) {
    if (chartVoltage) {
        point = chartVoltage.series[0].points[0];
        point.update(Math.round(newVal));
    }
}

var initGauges = function() {


    var initCurrentGauge = function() {
        var gaugeOptions = {
            chart: {
                type: 'solidgauge'
            },

            title: null,

            pane: {
                center: ['50%', '85%'],
                size: '140%',
                startAngle: -90,
                endAngle: 90,
                background: {
                    backgroundColor:
                        Highcharts.defaultOptions.legend.backgroundColor || '#EEE',
                    innerRadius: '60%',
                    outerRadius: '100%',
                    shape: 'arc'
                }
            },

            exporting: {
                enabled: false
            },

            tooltip: {
                enabled: false
            },

            // the value axis
            yAxis: {
                stops: [
                    [0.9, '#55BF3B'], // green
                    [0.8, '#DDDF0D'], // yellow
                    [0.1, '#DF5353'] // red
                ],
                lineWidth: 0,
                tickWidth: 0,
                minorTickInterval: null,
                tickAmount: 2,
                title: {
                    y: -70
                },
                labels: {
                    y: 16
                }
            },

            plotOptions: {
                solidgauge: {
                    dataLabels: {
                        y: 5,
                        borderWidth: 0,
                        useHTML: true
                    }
                }
            }
        };

        chartCurrent = Highcharts.chart('container-current', Highcharts.merge(gaugeOptions, {
            yAxis: {
                min: 0,
                max: 100,
                title: {
                    text: 'Current'
                }
            },

            credits: {
                enabled: false
            },

            series: [{
                name: 'Current',
                data: [0],
                dataLabels: {
                    format:
                        '<div style="text-align:center">' +
                        '<span style="font-size:25px">{y}</span><br/>' +
                        '<span style="font-size:12px;opacity:0.4">A</span>' +
                        '</div>'
                },
                tooltip: {
                    valueSuffix: ' A'
                }
            }]

        }));
    }

    var initVoltageGauge = function() {
        var gaugeOptions = {
            chart: {
                type: 'solidgauge'
            },

            title: null,

            pane: {
                center: ['50%', '85%'],
                size: '140%',
                startAngle: -90,
                endAngle: 90,
                background: {
                    backgroundColor:
                        Highcharts.defaultOptions.legend.backgroundColor || '#EEE',
                    innerRadius: '60%',
                    outerRadius: '100%',
                    shape: 'arc'
                }
            },

            exporting: {
                enabled: false
            },

            tooltip: {
                enabled: false
            },

            // the value axis
            yAxis: {
                stops: [
                    [0.1, '#DF5353'], // red
                    [0.3, '#DDDF0D'], // yellow
                    [0.4, '#55BF3B'] // green
                ],
                lineWidth: 0,
                tickWidth: 0,
                minorTickInterval: null,
                tickAmount: 2,
                title: {
                    y: -70
                },
                labels: {
                    y: 16
                }
            },

            plotOptions: {
                solidgauge: {
                    dataLabels: {
                        y: 5,
                        borderWidth: 0,
                        useHTML: true
                    }
                }
            }
        };

        chartVoltage = Highcharts.chart('container-voltage', Highcharts.merge(gaugeOptions, {
            yAxis: {
                min: 0,
                max: 100,
                title: {
                    text: 'Voltage'
                }
            },

            credits: {
                enabled: false
            },

            series: [{
                name: 'Voltage',
                data: [10],
                dataLabels: {
                    format:
                        '<div style="text-align:center">' +
                        '<span style="font-size:25px">{y}</span><br/>' +
                        '<span style="font-size:12px;opacity:0.4">V</span>' +
                        '</div>'
                },
                tooltip: {
                    valueSuffix: 'V'
                }
            }]

        }));
    }

    var initPowerGauge = function() {
        var gaugeOptions = {
            chart: {
                type: 'solidgauge'
            },

            title: null,

            pane: {
                center: ['50%', '85%'],
                size: '140%',
                startAngle: -90,
                endAngle: 90,
                background: {
                    backgroundColor:
                        Highcharts.defaultOptions.legend.backgroundColor || '#EEE',
                    innerRadius: '60%',
                    outerRadius: '100%',
                    shape: 'arc'
                }
            },

            exporting: {
                enabled: false
            },

            tooltip: {
                enabled: false
            },

            // the value axis
            yAxis: {
                stops: [
                    [0.9, '#55BF3B'], // green
                    [0.8, '#DDDF0D'], // yellow
                    [0.1, '#DF5353'] // red
                ],
                lineWidth: 0,
                tickWidth: 0,
                minorTickInterval: null,
                tickAmount: 2,
                title: {
                    y: -70
                },
                labels: {
                    y: 16
                }
            },

            plotOptions: {
                solidgauge: {
                    dataLabels: {
                        y: 5,
                        borderWidth: 0,
                        useHTML: true
                    }
                }
            }
        };

        chartPower = Highcharts.chart('container-power', Highcharts.merge(gaugeOptions, {
            yAxis: {
                min: 0,
                max: 100,
                title: {
                    text: 'Power'
                }
            },

            credits: {
                enabled: false
            },

            series: [{
                name: 'Power',
                data: [0],
                dataLabels: {
                    format:
                        '<div style="text-align:center">' +
                        '<span style="font-size:25px">{y}</span><br/>' +
                        '<span style="font-size:12px;opacity:0.4">KW</span>' +
                        '</div>'
                },
                tooltip: {
                    valueSuffix: ' KW'
                }
            }]

        }));
    }

    var initSocGauge = function() {
        var gaugeOptions = {
            chart: {
                type: 'solidgauge'
            },

            title: null,

            pane: {
                center: ['50%', '85%'],
                size: '140%',
                startAngle: -90,
                endAngle: 90,
                background: {
                    backgroundColor:
                        Highcharts.defaultOptions.legend.backgroundColor || '#EEE',
                    innerRadius: '60%',
                    outerRadius: '100%',
                    shape: 'arc'
                }
            },

            exporting: {
                enabled: false
            },

            tooltip: {
                enabled: false
            },

            // the value axis
            yAxis: {
                stops: [
                  [0.2, '#DF5353'], // red
                  [0.6, '#DDDF0D'], // yellow
                  [0.8, '#55BF3B'] // green
                ],
                lineWidth: 0,
                tickWidth: 0,
                minorTickInterval: null,
                tickAmount: 2,
                title: {
                    y: -70
                },
                labels: {
                    y: 16
                }
            },

            plotOptions: {
                solidgauge: {
                    dataLabels: {
                        y: 5,
                        borderWidth: 0,
                        useHTML: true
                    }
                }
            }
        };

        chartSoc = Highcharts.chart('container-soc', Highcharts.merge(gaugeOptions, {
            yAxis: {
                min: 0,
                max: 100,
                title: {
                    text: 'SoC'
                }
            },

            credits: {
                enabled: false
            },

            series: [{
                name: 'SoC',
                data: [0],
                dataLabels: {
                    format:
                        '<div style="text-align:center">' +
                        '<span style="font-size:25px">{y}</span><br/>' +
                        '<span style="font-size:12px;opacity:0.4">%</span>' +
                        '</div>'
                },
                tooltip: {
                    valueSuffix: ' %'
                }
            }]

        }));
    }

    initVoltageGauge();
    initCurrentGauge();
    initPowerGauge();
    initSocGauge();

}
