const request = require('request-promise-native');

const { hostTempStation, hostInfluxDB } = require('./config');

function log (data) {
  process.stdout.write(`${(new Date).toJSON().slice(11,19)} `);
  console.log(data);
}

log(`TempStation: ${hostTempStation}`);
log(`InfluxDB: ${hostInfluxDB}`);

// [{"P14":27.6},{"P12":26.9},{"P13":27.4},{"P0":27.3}]
const main = async () => {
  try {
    let data = await request(`http://${hostTempStation}`);
    data = JSON.parse(data);
    let metrics = [];
    for (let d of data) {
      let [k, v] = Object.entries(d)[0];
      const m = `distilmon,pin=${k} value=${v}`;
      metrics.push(m);
    }

    const opts = {
      headers: {
        'content-type': 'application/x-www-form-urlencoded',
      },
      body: metrics.join("\n"),
    };
    const result = await request.post(`http://${hostInfluxDB}:8086/write?db=climate`, opts);
    // log(`metrics: \n${metrics.join('\n')}`);
  } catch (e) {
    log(e);
  }
}

main();
setInterval(main, 5 * 1000);