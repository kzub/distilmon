const request = require('request-promise-native');

const { hostRegStation, hostInfluxDB } = require('./config');

function log (data) {
  process.stdout.write(`${(new Date).toJSON().slice(11,19)} `);
  console.log(data);
}

log(`RegStation: ${hostRegStation}`);
log(`InfluxDB: ${hostInfluxDB}`);

const checkValues = process.argv.slice(2);
if (!checkValues.length) {
  console.log("use check values format: P1:77 P2:78 ...");
  return
}
log(`Checking values: ${checkValues} `)

let query =  'SELECT mean("value") FROM "distilmon" WHERE time >= now() - 1m GROUP BY time(10s), "pin" fill(null)';
/* response example:
{
    "results": [
        {
            "statement_id": 0,
            "series": [
                {
                    "name": "distilmon",
                    "tags": {
                        "pin": "\"P0\""
                    },
                    "columns": [
                        "time",
                        "mean"
                    ],
                    "values": [
                        [
                            "2022-02-22T01:46:30Z",
                            34.236000000000004
                        ],
                        [
                            "2022-02-22T01:47:00Z",
                            34.18
                        ]
                    ]
                },
                {
                    "name": "distilmon",
                    "tags": {
                        "pin": "\"P12\""
                    },
                    "columns": [
                        "time",
                        "mean"
                    ],
                    "values": [
                        [
                            "2022-02-22T01:46:30Z",
                            22.75
                        ],
                        [
                            "2022-02-22T01:47:00Z",
                            22.75
                        ]
                    ]
                },
                {
                    "name": "distilmon",
                    "tags": {
                        "pin": "\"P14\""
                    },
                    "columns": [
                        "time",
                        "mean"
                    ],
                    "values": [
                        [
                            "2022-02-22T01:46:30Z",
                            23.407999999999998
                        ],
                        [
                            "2022-02-22T01:47:00Z",
                            23.28
                        ]
                    ]
                }
            ]
        }
    ]
}*/

// query='SELECT mean("value") FROM "distilmon" WHERE time >= 1645494393105ms and time <= 1645509576574ms GROUP BY time(5s), "pin" fill(null)';

function isPinTempOK(series, name, maxValue) {
	const values = series.filter(s => s.tags.pin == `${name}`).pop()?.values.slice(-2).map(v => v[1]);
	if (!values) {
		log(`No values for ${name}`);
		return false;
	}
	let lastValue = values[values.length - 1];
	log(`${name}: ${lastValue || 'null'}, history:[${values.join()}], Max: ${maxValue}`);
	if (lastValue == null) {
		if(values.filter(v => v == null).length == 2){
			log(`Null values for ${name}`);
			return false;
		}
		lastValue = values[0];
	}
	const res = lastValue <= maxValue;
	if (!res) {
		log(`Max value reached for ${name}, Value: ${lastValue}, Max: ${maxValue}`);
	}
	return res
}

async function downcreasePower() {
	let data = await request(`http://${hostRegStation}/rotate?steps=-175`);
	log(`New power level: ${data}`);
};

const main = async () => {
  try {
    let data = await request(`http://${hostInfluxDB}:8086/query?db=climate&q=${query}`);
    data = JSON.parse(data);

    const results = checkValues.map((check) => {
    	const [pin, maxValue] = check.split(':');
    	return isPinTempOK(data.results[0].series, pin, maxValue);
	});

    if (results.some(r => r == false)) {
    	await downcreasePower();
    } else {
      log('temp ok');
    }
  } catch (e) {
    log(e)
  }
}

main();
setInterval(main, 15 * 1000);


