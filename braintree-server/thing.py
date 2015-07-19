#!/usr/bin/env python3

import os

import braintree

print(braintree)

braintree.Configuration.configure(braintree.Environment.Sandbox,
                                  merchant_id="855p4hj8rkrnc5sd",
                                  public_key="9k8zgmzxbfqyfz37",
                                  private_key="78250b73760b6dbd2d97d4e04cdd7f93")

                
import flask
app = flask.Flask(__name__)

@app.route("/client_token", methods=["GET"])
def client_token():
    return braintree.ClientToken.generate()

@app.route("/payment-methods", methods=["POST"])
def create_purchase():
    nonce = request.form["payment_method_nonce"]
    result = braintree.Transaction.sale({
        "amount": "20.00",
        "payment_method_nonce": nonce
    })
    return result.transaction.id

if __name__ == "__main__":
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port)
